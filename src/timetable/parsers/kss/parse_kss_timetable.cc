#include "soro/timetable/parsers/kss/parse_kss_timetable.h"

#include <cctype>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "pugixml.hpp"

#include "date/date.h"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/erase_if.h"
#include "utl/logging.h"
#include "utl/parser/buf_reader.h"
#include "utl/parser/cstr.h"
#include "utl/parser/csv_range.h"
#include "utl/parser/line_range.h"
#include "utl/pipes/for_each.h"
#include "utl/timer.h"
#include "utl/verify.h"

#include "soro/base/error.h"
#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/file/loaded_file.h"
#include "soro/utls/narrow.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/result.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/any_of.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/string.h"

#include "soro/si/units.h"

#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/rolling_stock/parsers/iss/parse_stop_mode.h"
#include "soro/rolling_stock/rolling_stock.h"
#include "soro/rolling_stock/safety_systems.h"
#include "soro/rolling_stock/stop_mode.h"
#include "soro/rolling_stock/train_category.h"
#include "soro/rolling_stock/train_class.h"
#include "soro/rolling_stock/train_physics.h"
#include "soro/rolling_stock/train_series.h"

#include "soro/timetable/base_timetable.h"
#include "soro/timetable/bitfield.h"
#include "soro/timetable/connection.h"
#include "soro/timetable/interval.h"
#include "soro/timetable/parsers/kss/kss_error.h"
#include "soro/timetable/parsers/station_route_to_interlocking_route.h"
#include "soro/timetable/sequence_point.h"
#include "soro/timetable/timetable_options.h"
#include "soro/timetable/train.h"

namespace soro::tt {

using namespace pugi;
using namespace soro::rs;
using namespace soro::utls;
using namespace soro::infra;

namespace fs = std::filesystem;

bool is_kss_file(fs::path const& p) {
  return !fs::is_directory(p) && p.filename().extension() == ".xml" &&
         p.filename().string().starts_with("KSS-");
}

bool is_kss_timetable(timetable_options const& opts) {
  if (!std::filesystem::is_directory(opts.timetable_path_)) {
    return false;
  }

  // at least one kss file is required to be qualified as a kss timetable
  return utls::any_of(
      fs::directory_iterator{opts.timetable_path_},
      [](auto&& dir_entry) { return is_kss_file(dir_entry.path()); });
}

std::size_t distance(date::year_month_day const from,
                     date::year_month_day const to) {
  utls::sassert(from.ok(), "date from {} is not ok.", from);
  utls::sassert(to.ok(), "date to {} is not ok.", to);
  utls::sassert(from <= to,
                "call distance for year_month_date with ascending dates, "
                "got called with {} and {}",
                from, to);

  return static_cast<std::size_t>(
      (date::sys_days{to} - date::sys_days{from}).count());
}

std::set<train::number> parse_manually_removed(
    std::filesystem::path const& manually_csv) {
  utl::scoped_timer const timer("parsing manually removed");

  if (!fs::exists(manually_csv)) return {};

  std::set<train::number> result;

  struct row {
    utl::csv_col<train::number::main_t, UTL_NAME("main")> main_;
    utl::csv_col<train::number::sub_t, UTL_NAME("sub")> sub_;
    utl::csv_col<utl::cstr, UTL_NAME("reason")> reason_;
  };

  auto const manually_removed = utls::read_file_to_string(manually_csv);
  utl::line_range{utl::buf_reader{manually_removed}} | utl::csv<row>() |
      utl::for_each(
          [&](auto&& row) { result.emplace(row.main_.val(), row.sub_.val()); });

  return result;
}

bitfield parse_services(xml_node const services_xml) {
  std::size_t total_service_days = 0;

  auto const parse_special_service = [&](bitfield& bf,
                                         xml_node const special_service_xml) {
    auto const special_xml = special_service_xml.child("special");

    auto const date = parse_date(special_xml.attribute("date").value());

    if (utls::equal(special_xml.attribute("type").value(), "include")) {
      ++total_service_days;
      bf.set(date, true);
    } else if (utls::equal(special_xml.attribute("type").value(), "exclude")) {
      --total_service_days;
      bf.set(date, false);
    } else {
      uLOG(utl::warn) << "Found " << special_xml.attribute("type").value()
                      << " special service type but case is not implemented";
    }
  };

  bitfield accumulated_bitfield;
  utls::sassert(!accumulated_bitfield.ok());

  for (auto const service_xml : services_xml.children("service")) {
    if (auto bitmask_xml = service_xml.attribute("bitMask"); bitmask_xml) {
      auto const start_date =
          parse_date(service_xml.attribute("startDate").value());
      auto const end_date =
          parse_date(service_xml.attribute("endDate").value());
      auto const bitmask = service_xml.attribute("bitMask").value();

      utls::sasserts([&]() {
        auto const bitmask_length = strlen(bitmask);
        auto const dist = distance(start_date, end_date);

        utls::sassert(bitmask_length == dist + 1,
                      "Bitmask has not the same size as distance between start "
                      "and end date!");

        total_service_days += static_cast<std::size_t>(
            std::count(bitmask, bitmask + bitmask_length, '1'));
      });

      auto const bf = make_bitfield(start_date, end_date, bitmask);

      accumulated_bitfield.ok() ? accumulated_bitfield |= bf
                                : accumulated_bitfield = bf;
    } else {
      utls::sassert(
          accumulated_bitfield.ok(),
          "trying to set special date, but base bitfield is not constructed");
      parse_special_service(accumulated_bitfield, service_xml);
    }
  }

  utls::sassert(accumulated_bitfield.ok());
  utls::sassert(total_service_days == accumulated_bitfield.count(),
                "counted different amount of service days than ended up in the "
                "bitfield!");

  return accumulated_bitfield;
}

relative_time parse_time_offset(xml_node const time_xml) {
  auto const time_str = time_xml.child_value("time");

  utls::sassert(strlen(time_str) == strlen("HH:MM:SS.SS"));

  hours const h{parse_int<uint64_t>(time_str, time_str + 2)};
  minutes const m{parse_int<uint64_t>(time_str + 3, time_str + 5)};
  seconds const s{parse_int<uint64_t>(time_str + 6, time_str + 8)};

  seconds result = h + m + s;

  if (static_cast<bool>(time_xml.child("nightLeap"))) {
    result += sc::duration_cast<seconds>(days{1});
  }

  return std::chrono::duration_cast<relative_time>(result);
}

duration parse_duration(char const* const dur_str) {
  utls::sassert(dur_str[0] == 'P');
  utls::sassert(dur_str[1] == 'T');

  // Example duration strings:
  // PT_
  // PT6S_
  // PT5M_
  // PT5M6S_
  // PT5M54S_
  // PT15M54S_
  // PT15M54.99S_

  auto const str_end = dur_str + strlen(dur_str);

  auto letter_it = dur_str + 2;

  seconds result = seconds::zero();

  while (letter_it != str_end) {
    auto next_cap = std::find_if(letter_it, str_end, [](char const c) {
      return (std::isalpha(c) != 0) && (std::isupper(c) != 0);
    });

    switch (*next_cap) {
      case 'H': {
        result += hours{parse_int<hours::rep>(letter_it, next_cap)};
        break;
      }
      case 'M': {
        result += minutes{parse_int<minutes::rep>(letter_it, next_cap)};
        break;
      }
      case 'S': {
        result += seconds{static_cast<seconds::rep>(
            std::round(utls::parse_fp<double>(letter_it, next_cap)))};
        break;
      }
      default: {
        throw utl::fail("unexpected duration letter: {}", *next_cap);
      }
    }

    letter_it = next_cap + 1;
  }

  return std::chrono::duration_cast<duration>(result);
}

template <typename Type>
station_route::idx lookup_by_name(std::string_view const name, type const type,
                                  station_route::id const sr_id,
                                  infrastructure const& infra) {
  auto const& station_route = infra->station_routes_[sr_id];

  station_route::idx result = station_route::invalid_idx();

  for (auto const [idx, node] :
       utl::enumerate<station_route::idx>(station_route->nodes())) {
    if (!node->is(type)) continue;

    auto const e = node->element_;

    auto const& data = infra->graph_.get_element_data<Type>(e);

    // TODO(julian) lookup by name is wrong, but we currently don't have ids
    if (!utls::equal(data.name_, name)) continue;

    utls::ensure(idx < station_route->size(), "idx out of bounds");
    utls::ensure(station_route->nodes(idx)->element_ == e, "element mismatch");

    // name is not unique, we cannot determine the correct node via name
    if (result != station_route::invalid_idx()) {
      return station_route::invalid_idx();
    }

    result = idx;
  }

  return result;
}

utls::result<sequence_point> parse_additional_stop(
    xml_node const additional_stop_xml, station_route::id const sr,
    infrastructure const& infra) {
  sequence_point sp;

  sp.type_ = sequence_point::type::ADDITIONAL;
  sp.station_route_ = sr;

  auto element_xml = additional_stop_xml.child("infrastructureElement");
  if (!static_cast<bool>(element_xml)) {
    return utls::unexpected(error::kss::NO_ELEMENT_IN_ADDITIONAL_STOP);
  }

  auto const station_it =
      infra->ds100_to_station_.find(element_xml.attribute("DS100").value());

  if (station_it == std::end(infra->ds100_to_station_)) {
    return utls::unexpected(error::kss::STATION_NOT_FOUND_IN_ADDITIONAL_STOP);
  }

  auto const type = get_type(element_xml.attribute("Typ").value());
  auto const name = element_xml.child_value();

  switch (type) {
    case type::MAIN_SIGNAL: {
      sp.idx_ = lookup_by_name<main_signal>(name, type, sr, infra);
      break;
    }

    case type::HALT: {
      sp.idx_ = lookup_by_name<halt>(name, type, sr, infra);
      break;
    }

    case type::APPROACH_SIGNAL: {
      sp.idx_ = lookup_by_name<approach_signal>(name, type, sr, infra);
      break;
    }

    default: {
      return utls::unexpected(error::kss::ADDITIONAL_STOP_TYPE_NOT_SUPPORTED);
    }
  }

  if (sp.idx_ == station_route::invalid_idx()) {
    return utls::unexpected(error::kss::ADDITIONAL_STOP_NAME_NOT_FOUND);
  }

  auto const duration_xml = additional_stop_xml.child("additionalStopTime");
  sp.min_stop_time_ = static_cast<bool>(duration_xml)
                          ? parse_duration(duration_xml.child_value())
                          : duration::zero();

  return sp;
}

utls::result<station_route::idx> get_node_idx(station_route::ptr const sr,
                                              sequence_point::type const type,
                                              rs::stop_mode const mode) {
  utls::expect(type != sequence_point::type::INVALID, "got invalid type");

  auto const idx = type == sequence_point::type::TRANSIT
                       ? sr->get_runtime_checkpoint_idx()
                       : sr->get_halt_idx(mode);

  if (!idx.has_value()) {
    return utls::unexpected(error::kss::COULD_NOT_RESOLVE_SEQUENCE_POINT_NODE);
  }

  return *idx;
}

bool parse_boolean(std::string_view const bool_str) {
  using namespace std::literals;
  utls::expect((bool_str == "true"sv) || (bool_str == "false"sv),
               "bool string was neither 'true' nor 'false', but {}", bool_str);
  return bool_str == "true"sv;
}

bool parse_breaking_in(xml_node const sequence_xml) {
  return parse_boolean(sequence_xml.child_value("breakin"));
}

bool parse_breaking_out(xml_node const sequence_xml) {
  return parse_boolean(sequence_xml.child_value("breakout"));
}

si::speed parse_start_speed(xml_node const sequence_xml) {
  auto const start_xml = sequence_xml.child("startingVelocity");

  if (!static_cast<bool>(start_xml)) return si::speed::zero();

  auto const v = utls::parse_fp<si::speed::precision>(start_xml.child_value());

  return si::from_km_h(v);
}

std::optional<rs::stop_mode> try_parsing_stop_mode(
    xml_node const stop_mode_xml) {
  if (!static_cast<bool>(stop_mode_xml)) return std::nullopt;

  return parse_stop_mode(stop_mode_xml);
}

utls::result<soro::vector<sequence_point>> parse_sequence_points(
    xml_node const sequence_xml, infrastructure const& infra,
    stop_mode const stop_mode) {
  soro::vector<sequence_point> result;

  auto const sequence_points_xml = sequence_xml.child("sequenceServicePoints");
  for (auto const point_xml : sequence_points_xml.children()) {
    sequence_point sp;

    std::string_view const ds100{point_xml.child_value("servicePoint")};
    std::string_view const route_name{point_xml.child_value("trackSystem")};

    auto station_it = infra->ds100_to_station_.find(ds100);
    if (station_it == std::end(infra->ds100_to_station_)) {
      return utls::unexpected(error::kss::STATION_NOT_FOUND);
    }

    auto route_it = station_it->second->station_routes_.find(route_name);
    if (route_it == std::end(station_it->second->station_routes_)) {
      return utls::unexpected(error::kss::STATION_ROUTE_NOT_FOUND);
    }
    sp.station_route_ = route_it->second->id_;

    if (auto const arr = point_xml.child("arrival"); arr) {
      sp.arrival_ = soro::optional<relative_time>{parse_time_offset(arr)};
    }

    if (auto const dep = point_xml.child("departure"); dep) {
      sp.departure_ = soro::optional<relative_time>{parse_time_offset(dep)};
    }

    if (sp.departure_.has_value() && !sp.arrival_.has_value()) {
      sp.arrival_ = sp.departure_;
    }

    // we will call it stop type, since stop mode is already being used
    // to discern between freight and passenger stops
    auto const stop_type_xml = point_xml.child("stopMode");
    if (static_cast<bool>(stop_type_xml)) {
      sp.type_ =
          KEY_TO_STOP_TYPE.at(*stop_type_xml.attribute("Schluessel").value());
    } else {
      sp.type_ = sequence_point::type::TRANSIT;
    }

    auto const alt_stop_mode =
        try_parsing_stop_mode(point_xml.child("relevantStopPositionMode"));

    // do this after resolving the type and the arrival/departure values
    if (sp.arrival_.has_value() || sp.departure_.has_value()) {
      auto const sr = infra->station_routes_[sp.station_route_];
      auto const used_stop_mode = alt_stop_mode.value_or(stop_mode);
      auto const node_idx = get_node_idx(sr, sp.type_, used_stop_mode);
      if (!node_idx) return utls::propagate(node_idx);
      sp.idx_ = *node_idx;
    }

    auto const min_stop_time_xml = point_xml.child("minStopTime");
    if (static_cast<bool>(min_stop_time_xml)) {
      sp.min_stop_time_ = parse_duration(min_stop_time_xml.child_value());
    } else {
      sp.min_stop_time_ = duration::zero();
    }

    auto additional_stop_xml = point_xml.child("additionalStop");
    if (!static_cast<bool>(additional_stop_xml)) {
      result.emplace_back(sp);
      continue;
    }

    // handle the additional stop
    auto const as =
        parse_additional_stop(additional_stop_xml, sp.station_route_, infra);
    if (!as) return utls::propagate(as);

    utls::sassert(as->idx_ != station_route::invalid_idx(), "as without node");
    utls::sassert(as->idx_ != sp.idx_, "two halts at same node?");

    auto const insert_as_before_sp = as->idx_ < sp.idx_;

    if (insert_as_before_sp) {
      result.emplace_back(*as);
      result.emplace_back(sp);
    } else {
      result.emplace_back(sp);
      result.emplace_back(*as);
    }
  }

  return result;
}

rs::train_class get_train_class(xml_node const& train_class_xml,
                                rs::rolling_stock const& rolling_stock) {
  auto const key =
      parse_dictionary_key<rs::train_class::key>(train_class_xml.child_value());
  auto const it = rolling_stock.train_classes_.find(key);

  if (it == std::end(rolling_stock.train_classes_)) {
    throw utl::fail("could not determine train class from key {}", key);
  }

  return it->second;
}

rs::train_category get_train_category(xml_node const& train_category_xml,
                                      rs::rolling_stock const& rolling_stock) {
  train_category::key key;

  auto const main_val = train_category_xml.attribute("Hauptnummer").value();
  key.main_ = train_category::key::main{
      utls::parse_int<train_category::key::main::value_t>(main_val)};

  auto const sub_val = train_category_xml.child_value();
  key.sub_ = train_category::key::sub{
      utls::parse_int<train_category::key::sub::value_t>(sub_val)};

  key.name_ = train_category_xml.attribute("ZuggattungsProdukt").value();

  auto const it = rolling_stock.train_categories_.find(key);

  if (it == std::end(rolling_stock.train_categories_)) {
    throw utl::fail("could not determine train category from key {}.{} {}",
                    key.main_, key.sub_, key.name_);
  }

  return it->second;
}

soro::vector<rs::transportation_specialty> get_specialties(
    xml_node const& specials_xml, dictionaries const& dicts) {
  soro::vector<rs::transportation_specialty> result;

  for (auto const& specialty_xml : specials_xml.children("carriageSpecific")) {
    // TODO(julian) parse transportation specialties correctly
    std::ignore = specialty_xml;
    std::ignore = dicts;
  }

  return result;
}

bearing_friction_coefficient parse_bearing_coefficient(
    xml_node const& charac_xml) {
  auto const bearing_xml = charac_xml.child("bearingFrictionCoefficient");

  auto const val =
      static_cast<bool>(bearing_xml)
          ? utls::parse_fp<rs::bearing_friction_coefficient::precision>(
                bearing_xml.child_value())
          : 1.4;

  // as the coefficient is given for ton, and we are using kg, divide by 1000
  return rs::bearing_friction_coefficient{val / 1000.0};
}

air_resistance_coefficient parse_air_resistance(xml_node const& charac_xml) {
  auto const air_xml = charac_xml.child("airResistanceCoefficient");

  auto const val =
      static_cast<bool>(air_xml)
          ? utls::parse_fp<rs::air_resistance_coefficient::precision>(
                air_xml.child_value())
          : 0.05;

  // since the constants 0.007 and 100 are for km/h and ton, and we use m/s and
  // kg we convert them by multiplying with 3.6^2 and dividing by 1000
  air_resistance_coefficient const l{
      (((val + 0.007) / 100) * std::pow(3.6, 2) / 1000)};

  return l;
}

utls::result<rs::train_physics> parse_characteristic(
    xml_node const charac_xml, infrastructure const& infra) {
  auto const traction_units_xml = charac_xml.child("tractionUnits");

  soro::vector<configured_traction_unit> ctus;

  for (auto const traction_unit_xml : traction_units_xml.children()) {
    auto const series_xml = traction_unit_xml.child("tractionUnitDesignSeries");

    /* TODO(julian) not all traction vehicles are active at all times
      auto const is_standard_traction_unit =
        parse_boolean(traction_unit_xml.child_value("isStandardTractionUnit"));
    */

    traction_unit::key tuk;

    auto const number_val = series_xml.child_value("designSeries");
    auto const company_val =
        series_xml.child("designSeries").attribute("Nr").value();
    auto const variant_val = utls::parse_int<train_series::variant::value_t>(
        series_xml.child_value("variante"));

    tuk.train_series_key_.number_ = train_series::key::number{number_val};
    tuk.train_series_key_.company_ = train_series::key::company{company_val};
    tuk.variant_ = train_series::variant{variant_val};

    auto const& tu = get_traction_vehicle(infra->rolling_stock_, tuk);
    if (!tu) return utls::propagate(tu);

    configured_traction_unit ctu;
    // copy, not move
    ctu.traction_unit_ = *tu;

    electric_configuration config;

    auto const config_xml = series_xml.child("stromart");
    auto const config_val = config_xml.attribute("Schluessel").value();

    if (!utls::equal(config_val, "0")) {
      auto const config_key =
          parse_dictionary_key<electric_configuration_key>(config_val);
      config.emplace(
          infra->dictionaries_.electric_configuration_.get_id(config_key));
    }

    utls::sassert(config.has_value() ||
                  utls::equal(config_xml.child_value(), "ohne Stromart"));

    ctu.selected_ = tu->get_equipment_idx(config);

    ctus.emplace_back(std::move(ctu));
  }

  si::length const tus_length = utls::accumulate(
      ctus, si::length::zero(),
      [](auto&& acc, auto&& ctu) { return acc + ctu.traction_unit_.length_; });

  auto const carriage_length = si::from_m(utls::parse_fp<si::precision>(
                                   charac_xml.child_value("totalLength"))) -
                               tus_length;

  si::weight const tus_weight = utls::accumulate(
      ctus, si::weight::zero(),
      [](auto&& acc, auto&& ctu) { return acc + ctu.traction_unit_.weight_; });

  auto const carriage_weight = si::from_ton(utls::parse_fp<si::precision>(
                                   charac_xml.child_value("totalWeight"))) -
                               tus_weight;

  auto const max_speed = si::from_km_h(
      utls::parse_fp<si::precision>(charac_xml.child_value("maxVelocity")));

  auto const cars_xml = charac_xml.child("numberofPassengerCars");
  auto const cars = static_cast<bool>(cars_xml)
                        ? utls::parse_int<soro::size_t>(cars_xml.child_value())
                        : 0;

  auto const lzb = static_cast<rs::LZB>(
      utls::equal(charac_xml.child_value("trainProtection"), "true"));

  auto const stop_mode =
      rs::parse_stop_mode(charac_xml.child("relevantStopPositionMode"));

  auto const bwp = rs::brake_weight_percentage{
      utls::parse_fp<rs::brake_weight_percentage::precision>(
          charac_xml.child_value("brakedWeightPercentage"))};

  soro::optional<tilt_technology> tilt;
  auto const& tilt_xml = charac_xml.child("bodyTiltingTechnique");
  if (static_cast<bool>(tilt_xml)) {
    auto const key_value = tilt_xml.attribute("Schluessel").value();
    auto const key = parse_dictionary_key<tilt_technology_key>(key_value);
    tilt.emplace(infra->dictionaries_.tilt_technology_.get_id(key));
  }

  auto const air = parse_air_resistance(charac_xml);

  auto const bearing = parse_bearing_coefficient(charac_xml);

  auto const brake_position_val =
      charac_xml.child("brakingSystem").attribute("Schluessel").value();
  auto const brake_position_key =
      parse_dictionary_key<rs::brake_position_key>(brake_position_val);
  auto const brake_position =
      infra->dictionaries_.brake_position_.get_id(brake_position_key);

  auto const brake_type_it =
      infra->dictionaries_.brake_position_to_brake_type_.find(brake_position);
  auto const brake_type = brake_type_it->second;

  auto const train_class =
      get_train_class(charac_xml.child("trainClass"), infra->rolling_stock_);

  auto const train_category =
      get_train_category(charac_xml.child("kind"), infra->rolling_stock_);

  auto const specialties = get_specialties(
      charac_xml.child("carriageSpecifics"), infra->dictionaries_);

  return train_physics{
      ctus,      carriage_weight, carriage_length, max_speed,  cars, lzb,
      stop_mode, brake_position,  brake_type,      bwp,        tilt, air,
      bearing,   train_class,     train_category,  specialties};
}

utls::result<void> is_supported(train const& train,
                                infrastructure const& infra) {
  if (train.has_lzb()) return utls::unexpected(error::kss::LZB_NOT_SUPPORTED);

  auto const last_sr_id = train.sequence_points_.back().station_route_;
  auto const& last_sr = infra->station_routes_[last_sr_id];
  if (train.break_out_ && !last_sr->nodes().back()->is(type::TRACK_END)) {
    return utls::unexpected(error::kss::BREAK_OUT_NOT_ENDING_ON_TRACK_END);
  }

  auto const first_sr_id = train.sequence_points_.front().station_route_;
  auto const& first_sr = infra->station_routes_[first_sr_id];
  if (train.break_in_ && !first_sr->nodes().front()->is(type::TRACK_END)) {
    return utls::unexpected(error::kss::BREAK_IN_NOT_STARTING_ON_TRACK_END);
  }

  if (train.sequence_points_.size() < 2) {
    return utls::unexpected(error::kss::SINGLE_STOP_NOT_SUPPORTED);
  }

  if (!train.sequence_points_.front().is_halt() && !train.break_in_) {
    return utls::unexpected(error::kss::FIRST_STOP_NO_HALT_NOT_SUPPORTED);
  }

  if (!train.sequence_points_.back().is_halt() && !train.break_out_) {
    return utls::unexpected(error::kss::LAST_STOP_NO_HALT_NOT_SUPPORTED);
  }

  return {};
}

train::number parse_train_number(xml_node const train_number_xml) {
  train::number tn;

  tn.main_ = utls::parse_int<train::number::main_t>(
      train_number_xml.child_value("mainNumber"));
  tn.sub_ = utls::parse_int<train::number::sub_t>(
      train_number_xml.child_value("subNumber"));

  return tn;
}

relative_time get_start_time_break_in(train const& t,
                                      infrastructure const& infra) {
  std::ignore = t;
  std::ignore = infra;
  return ZERO<relative_time>;

  //  auto const& first_point = t.sequence_points_.front();
  //
  //  utls::sassert(first_point.idx_ != station_route::invalid_idx(),
  //                "invalid node idx");
  //  utls::sassert(
  //      (first_point.is_halt() && first_point.arrival_.has_value()) ||
  //      (first_point.is_transit() && first_point.departure_.has_value()));
  //
  //  auto const first_time_stamp =
  //      (first_point.is_halt() ? *first_point.arrival_ :
  //      *first_point.departure_);
  //  utls::sassert(first_time_stamp != relative_time::max(),
  //                "base time is invalid");
  //
  //  auto const target = first_point.get_node(infra);
  //
  //  auto target_arrival = relative_time::max();
  //  auto const event_reached = [&](auto&& e) {
  //    if (e.element_->get_id() != target->element_->get_id()) return;
  //
  //    utls::sassert(target_arrival == relative_time::max(), "no overwriting");
  //    utls::sassert(e.arrival_ <= first_time_stamp, "integer underflow");
  //
  //    target_arrival = e.arrival_;
  //  };
  //
  //  auto const terminate = [&](auto&& n) {
  //    return target->get_id() == n->get_id();
  //  };
  //
  //  rk4::runtime_calculation(t, infra, {target->type()}, use_surcharge::yes,
  //                           event_reached, terminate);
  //
  //  utls::sassert(target_arrival != relative_time::max(),
  //                "could not find target arrival time");
  //
  //  auto const result = first_time_stamp - target_arrival;
  //
  //  utls::ensure(first_point.arrival_.has_value(), "no arrival value");
  //  utls::ensure(result <= *first_point.arrival_, "start must before end");
  //
  //  return result;
}

relative_time get_start_time_normal(train const& t) {
  auto const& first_point = t.sequence_points_.front();

  utls::sassert(first_point.idx_ != station_route::invalid_idx(),
                "idx invalid");
  utls::sassert(
      (first_point.is_halt() && first_point.arrival_.has_value()) ||
      (first_point.is_transit() && first_point.departure_.has_value()));

  utls::sassert(!first_point.is_transit(), "not supported");

  return *first_point.departure_;
}

relative_time get_start_time(train const& t, infrastructure const& infra) {
  utls::expect(!t.sequence_points_.empty(), "no sequence point");

  return t.break_in_ ? get_start_time_break_in(t, infra)
                     : get_start_time_normal(t);
}

relative_time get_end_time(train const& t, infrastructure const&) {
  // TODO(julian) this is not correct
  // for trains breaking out the distance between the last sequence point and
  // the exiting node is not covered
  // a runtime calculation is needed to determine the correct end time
  utls::expect(!t.sequence_points_.empty(), "no sequence point");
  auto const& last = t.sequence_points_.back();
  utls::sassert(last.departure_.has_value() || last.arrival_.has_value(),
                "no departure or arrival time");
  return last.departure_.has_value() ? *last.departure_ : *last.arrival_;
}

utls::result<train> parse_construction_train(
    xml_node const construction_train_xml,
    std::set<train::number> const& remove, infrastructure const& infra) {
  train t;

  t.number_ = parse_train_number(construction_train_xml.child("trainNumber"));

  if (remove.contains(t.number_)) {
    return utls::unexpected(error::kss::MANUALLY_REMOVED);
  }

  auto const charac_xml = construction_train_xml.child("characteristic");
  auto phys = parse_characteristic(charac_xml, infra);
  if (!phys) return utls::propagate(phys);
  t.physics_ = std::move(*phys);

  auto const seq_xml = construction_train_xml.child("sequence");

  auto const zlb_xml = seq_xml.child("ZLBNotes");
  if (static_cast<bool>(zlb_xml)) {
    return utls::unexpected(error::kss::ZLB_NOT_SUPPORTED);
  }

  auto const operational_notes_xml = seq_xml.child("operationalNotes");
  if (static_cast<bool>(operational_notes_xml)) {
    for (auto const& note_xml : operational_notes_xml.children()) {
      auto const tt = note_xml.child("templateTitle");
      if (utls::equal(tt.attribute("Kategorie").value(), "1") &&
          utls::equal(tt.attribute("Schluessel").value(), "VORZUG")) {
        return utls::unexpected(error::kss::OP_NOTES_NOT_SUPPORTED);
      }
    }
  }

  auto points = parse_sequence_points(seq_xml, infra, t.stop_mode());
  if (!points) return utls::propagate(points);
  t.sequence_points_ = std::move(*points);

  t.break_in_ = parse_breaking_in(seq_xml);
  t.break_out_ = parse_breaking_out(seq_xml);

  auto const supported = is_supported(t, infra);
  if (!supported) return utls::propagate(supported);

  // TODO(julian) what to do with start speed when train is not breaking in?
  t.start_speed_ = t.break_in_ ? parse_start_speed(seq_xml) : si::speed::zero();

  auto transformed = transform_to_interlocking(t, infra);
  if (!transformed) return utls::propagate(transformed);
  t.path_ = std::move(*transformed);

  // erase all sequence points without nodes, after we got the IR path
  utl::erase_if(t.sequence_points_, [](auto&& sp) {
    return sp.idx_ == station_route::invalid_idx();
  });

  t.start_time_ = get_start_time(t, infra);
  t.end_time_ = get_end_time(t, infra);

  t.service_days_ = parse_services(construction_train_xml.child("services"));

  return t;
}

void print_mode_stats(std::map<connection::mode, size_t> const& stats) {
  // print mode names and occurences
  for (auto const& [mode, count] : stats) {
    switch (mode) {
      case connection::mode::invalid: std::cout << "invalid: "; break;
      case connection::mode::continuation: std::cout << "continuation: "; break;
      case connection::mode::continuation_with_break:
        std::cout << "continuation_with_break: ";
        break;
      case connection::mode::uturn: std::cout << "uturn: "; break;
      case connection::mode::uturn_with_break:
        std::cout << "uturn_with_break: ";
        break;
      case connection::mode::through: std::cout << "through: "; break;
      case connection::mode::through_with_break:
        std::cout << "through_with_break: ";
        break;
      case connection::mode::circular: std::cout << "circular: "; break;
      case connection::mode::circular_with_break:
        std::cout << "circular_with_break: ";
        break;
      case connection::mode::park: std::cout << "park: "; break;
      case connection::mode::connection: std::cout << "connection: "; break;
      case connection::mode::trim: std::cout << "trim: "; break;
      case connection::mode::tact: std::cout << "tact: "; break;
      case connection::mode::alternative: std::cout << "alternative: "; break;
      case connection::mode::exclusion: std::cout << "exclusion: "; break;
    }

    std::cout << count << '\n';
  }
}

connection::mode parse_connection_mode(xml_node const connection_mode_xml) {
  switch (str_hash(connection_mode_xml.child_value())) {
    case str_hash("Direkte Fortsetzung"): return connection::mode::continuation;
    case str_hash("Nicht-direkte Fortsetzung"):
      return connection::mode::continuation_with_break;
    case str_hash("Direktes Kopfmachen"): return connection::mode::uturn;
    case str_hash("Nicht-direktes Kopfmachen"):
      return connection::mode::uturn_with_break;
    case str_hash("Direkte Durchbindung"): return connection::mode::through;
    case str_hash("Nicht-direkte Durchbindung"):
      return connection::mode::through_with_break;
    case str_hash("Direkter Umlauf"): return connection::mode::circular;
    case str_hash("Nicht-direkter Umlauf"):
      return connection::mode::circular_with_break;
    case str_hash("Abstellung"): return connection::mode::park;
    case str_hash("Anschluss"): return connection::mode::connection;
    case str_hash("Trimmung"): return connection::mode::trim;
    case str_hash("Takt"): return connection::mode::tact;
    case str_hash("Alternative"): return connection::mode::alternative;
    case str_hash("Ausschluss"): return connection::mode::exclusion;

    default: utls::sassert(false, "unknown connection mode");
  }

  std::unreachable();
}

utls::result<station::ptr> ds100_to_station(std::string_view const ds100,
                                            infrastructure const& infra) {
  auto const station_it = infra->ds100_to_station_.find(ds100);
  if (station_it == std::end(infra->ds100_to_station_)) {
    return utls::unexpected(error::kss::STATION_NOT_FOUND);
  }

  return station_it->second;
}

utls::result<connection> parse_connection(xml_node const connection_xml,
                                          infrastructure const& infra) {
  connection c;

  c.mode_ = parse_connection_mode(connection_xml.child("connectionMode"));

  c.first_train_number_ =
      parse_train_number(connection_xml.child("firstTrain"));
  c.second_train_number_ =
      parse_train_number(connection_xml.child("secondTrain"));

  auto const first_station =
      ds100_to_station(connection_xml.child_value("firstServicePoint"), infra);
  if (!first_station) return utls::propagate(first_station);
  c.first_station_ = (*first_station)->id_;

  // parse second station, if it is present
  if (auto second = connection_xml.child("secondServicePoint"); second) {
    auto const second_station = ds100_to_station(second.child_value(), infra);
    if (!second_station) return utls::propagate(second_station);
    c.second_station_ = soro::optional<station::id>{(*second_station)->id_};
  }

  if (auto time_xml = connection_xml.child("timeValue"); time_xml) {
    c.time_ = soro::optional<duration>{parse_duration(time_xml.child_value())};
  }

  return c;
}

soro::vector<connection> parse_connections(xml_node const fine_construction_xml,
                                           infrastructure const& infra) {
  soro::vector<connection> result;

  for (auto const conn_xml : fine_construction_xml.children("connection")) {
    if (!starts_with(conn_xml.child_value("status"), "Erf")) {
      continue;
    }

    auto const connection = parse_connection(conn_xml, infra);
    result.emplace_back(*connection);
  }

  return result;
}

void connect_continuation(soro::vector<train>& trains,
                          connection const& connection) {
  utls::expect(connection.mode_ == connection::mode::continuation ||
               connection.mode_ == connection::mode::continuation_with_break);

  auto const first_train = utls::find_if(trains, [&](auto&& t) {
    return t.number_ == connection.first_train_number_;
  });
  auto const second_train = utls::find_if(trains, [&](auto&& t) {
    return t.number_ == connection.second_train_number_;
  });

  utls::sassert(first_train != std::end(trains), "first train not found");
  utls::sassert(second_train != std::end(trains), "second train not found");

  //  utls::sassert(first_train->physics_ == second_train->physics_,
  //                "trains have different physics");
  utls::sassert(first_train->path_.back() == second_train->path_.front(),
                "trains are not connected");
  //  utls::sassert(first_train->sequence_points_.back() ==
  //                    second_train->sequence_points_.front(),
  //                "trains are not connected");
  utls::sassert(first_train->service_days_ == second_train->service_days_,
                "trains have different service days");
}

void connect_trains(soro::vector<train>& trains,
                    soro::vector<connection> const& connections) {

  for (auto const& connection : connections) {
    switch (connection.mode_) {
      case connection::mode::continuation:
        connect_continuation(trains, connection);
        break;
      default: break;
    }
  }
}

struct parsed_kss_train {
  soro::vector<train> trains_;
  soro::vector<connection> connections_;
};

utls::result<parsed_kss_train> parse_kss_train(
    xml_node const train_xml, std::set<train::number> const& remove,
    infrastructure const& infra) {
  parsed_kss_train result;

  // don't parse worked on trains
  auto const train_status = train_xml.attribute("trainStatus").value();
  if (!utls::equal(train_status, "freig")) {
    return utls::unexpected(error::kss::WORKED_ON_TRAIN);
  }

  auto const fine_construction_xml = train_xml.child("fineConstruction");

  for (auto const construction_train_xml :
       fine_construction_xml.children("constructionTrain")) {

    auto const train =
        parse_construction_train(construction_train_xml, remove, infra);

    if (!train) {
      return utls::propagate(train);
    }

    result.trains_.emplace_back(*train);
  }

  result.connections_ = parse_connections(fine_construction_xml, infra);

  return result;
}

struct timetable_file_parse_result {
  soro::vector<train> trains_;
  soro::vector<connection> connections_;
};

timetable_file_parse_result parse_timetable_file(
    std::filesystem::path const& fp, std::set<train::number> const& remove,
    infrastructure const& infra, error::stats& stats) {
  timetable_file_parse_result result;

  auto const loaded_file = utls::load_file(fp);

  xml_document file_xml;
  auto success = file_xml.load_buffer(
      reinterpret_cast<void const*>(loaded_file.data()), loaded_file.size());

  utl::verify(success,
              "pugixml error '{}' while parsing {} from kss timetable {}!",
              success.description(), fp);

  auto const timetable_xml =
      file_xml.child("KSS").child("railml").child("timetable");

  for (auto const train_xml : timetable_xml.children("train")) {
    auto const parsed_kss_train = parse_kss_train(train_xml, remove, infra);

    if (!parsed_kss_train) {
      stats.count(parsed_kss_train);
      continue;
    }

    utl::concat(result.trains_, parsed_kss_train->trains_);
    utl::concat(result.connections_, parsed_kss_train->connections_);
  }

  return result;
}

void set_ids(soro::vector<train>& trains) {
  for (auto [id, train] : utl::enumerate(trains)) {
    train.id_ = utls::narrow<train::id>(id);
  }
}

infra::version get_required_infra_version(timetable_options const& opts) {
  utls::expect(fs::is_directory(opts.timetable_path_),
               "timetable path {} is not a directory", opts.timetable_path_);
  utls::expect(!fs::is_empty(opts.timetable_path_),
               "timetable path {} is empty", opts.timetable_path_);

  auto const first_fp =
      fs::begin(fs::directory_iterator{opts.timetable_path_});  // NOLINT
  auto const first_file = utls::load_file(first_fp->path());

  xml_document tt_xml;
  auto success = tt_xml.load_buffer(
      reinterpret_cast<void const*>(first_file.data()), first_file.size());
  utl::verify(success,
              "pugixml error '{}' while parsing {} from kss timetable {}!",
              success.description(), *first_fp);

  auto const version_xml =
      tt_xml.child("KSS").child("header").child("spurplanVersionDescription");

  version v;

  v.name_ = version_xml.child_value("name");
  v.number_ =
      utls::parse_int<version::number>(version_xml.child_value("number"));

  return v;
}

interval get_interval(soro::vector<train> const& trains) {
  interval result{.start_ = absolute_time::max(), .end_ = absolute_time::min()};

  for (auto const& train : trains) {
    result.start_ = std::min(result.start_, train.first_absolute_timestamp());
    result.end_ = std::max(result.end_, train.last_absolute_timestamp());
  }

  return result;
}

std::map<train::number, std::vector<train::id>> get_train_number_to_ids(
    soro::vector<train> const& trains) {
  std::map<train::number, std::vector<train::id>> result;

  for (auto const& train : trains) {
    result[train.number_].emplace_back(train.id_);
  }

  return result;
}

utls::result<train::id> resolve_train_number(
    train::number const& number, soro::vector<train> const& trains,
    std::map<train::number, std::vector<train::id>> candidates) {

  std::ignore = trains;

  auto const& candidate_ids = candidates[number];

  //  utls::sassert(candidate_ids.size() == 1);

  if (candidate_ids.size() == 1) {
    return candidate_ids.front();
  }

  if (candidate_ids.empty()) {
    return utls::unexpected(error::kss::TRAIN_NUMBER_NOT_FOUND);
  }

  if (candidate_ids.size() > 1) {
    return utls::unexpected(error::kss::TRAIN_NUMBER_AMBIGUOUS);
  }

  std::unreachable();
}

void resolve_connections(soro::vector<connection>& connections,
                         soro::vector<train> const& trains,
                         error::stats& stats) {

  auto const train_number_to_ids = get_train_number_to_ids(trains);

  for (auto& conn : connections) {
    auto const first_id = resolve_train_number(conn.first_train_number_, trains,
                                               train_number_to_ids);
    if (!first_id) {
      stats.count(first_id);
      continue;
    }

    conn.first_train_ = *first_id;

    auto const second_id = resolve_train_number(conn.second_train_number_,
                                                trains, train_number_to_ids);
    if (!second_id) {
      stats.count(second_id);
      continue;
    }

    conn.first_train_ = *second_id;
  }

  uLOG(utl::info) << "total connections: " << connections.size();

  utl::erase_if(connections, [](auto const& conn) {
    return conn.first_train_ == train::invalid() ||
           conn.second_train_ == train::invalid();
  });

  uLOG(utl::info) << "resolved connections: " << connections.size();
}

utls::result<base_timetable> parse_kss_timetable(
    timetable_options const& opts, infra::infrastructure const& infra) {

  // TODO(julian) maybe reuse the options here?
  utls::expect(!infra->interlocking_.routes_.empty(),
               "interlocking routes required for timetable parsing");

  auto const required_version = get_required_infra_version(opts);

  if (required_version != infra->version_) {
    return utls::unexpected(error::kss::INFRASTRUCTURE_VERSION_MISMATCH);
  }

  utl::scoped_timer const timetable_timer("parsing kss timetable from " +
                                          opts.timetable_path_.string());

  base_timetable bt;
  bt.source_ = opts.timetable_path_.filename().string();

  error::stats stats("parsing kss timetable");

  auto const manually_removed =
      parse_manually_removed(opts.timetable_path_ / "manually_removed.csv");

  struct work_item {
    fs::path timetable_file_;
    timetable_file_parse_result result_;
  };

  std::vector<work_item> work_todo;

  for (auto const& dir_entry : fs::directory_iterator{opts.timetable_path_}) {
    if (!is_kss_file(dir_entry.path())) {
      continue;
    }

    work_todo.emplace_back(work_item{dir_entry.path(), {}});
  }

  //  utl::parallel_for_run(work_todo.size(), [&](auto&& work_id) {
  //    work_todo[work_id].result_ = parse_timetable_file(
  //        work_todo[work_id].timetable_file_, manually_removed, infra, stats);
  //  });

  // TODO(julian) maybe a parallel for where the thread count can be given as
  // parameter is in order
  /* kept around for debug purposes ... */
  for (auto& work_item : work_todo) {
    work_item.result_ = parse_timetable_file(work_item.timetable_file_,
                                             manually_removed, infra, stats);
  }

  for (auto const& work_item : work_todo) {
    utl::concat(bt.trains_, work_item.result_.trains_);
    utl::concat(bt.connections_, work_item.result_.connections_);
  }

  set_ids(bt.trains_);

  uLOG(utl::info) << "total trains successfully parsed: " << bt.trains_.size();
  stats.report();

  bt.interval_ = get_interval(bt.trains_);

  return bt;
}

}  // namespace soro::tt
