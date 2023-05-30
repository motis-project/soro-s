#include "soro/timetable/parsers/kss/parse_kss_timetable.h"

#include <string_view>

#include "pugixml.hpp"

#include "soro/rolling_stock/freight.h"
#include "soro/timetable/sequence_point.h"
#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/erase_if.h"
#include "utl/logging.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/base/error.h"

#include "soro/utls/narrow.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/statistics.h"
#include "soro/utls/std_wrapper/any_of.h"
#include "soro/utls/string.h"

#include "soro/timetable/bitfield.h"
#include "soro/timetable/parsers/kss/kss_error.h"
#include "soro/timetable/parsers/station_route_to_interlocking_route.h"

namespace soro::tt {

using namespace pugi;
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

bitfield parse_services(xml_node const services_xml) {
  std::size_t total_service_days = 0;

  auto const parse_date = [](const char* const c) {
    utls::sassert(strlen(c) == strlen("2022-11-19"),
                  "Date {} has not the expected length.", c);

    date::year const y{parse_int<int>(c, c + 4)};
    date::month const m{parse_int<unsigned>(c + 5, c + 7)};
    date::day const d{parse_int<unsigned>(c + 8, c + 10)};

    date::year_month_day date{y, m, d};

    utls::sassert(date.ok(), "Date {} - {} - {} is not ok!", date.year(),
                  date.month(), date.day());

    return date;
  };

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
          "Trying to set special date, but base bitfield is not constructed");
      parse_special_service(accumulated_bitfield, service_xml);
    }
  }

  utls::sassert(accumulated_bitfield.ok());
  utls::sassert(total_service_days == accumulated_bitfield.count(),
                "Counted different amount of service days than ended up in the "
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

duration2 parse_duration(char const* const dur_str) {
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
    }

    letter_it = next_cap + 1;
  }

  return std::chrono::duration_cast<duration2>(result);
}

bool parse_boolean(std::string_view const bool_str) {
  using namespace std::literals;
  utls::expect(equal(bool_str, "true"sv) || equal(bool_str, "false"sv),
               "bool string was neither 'true' nor 'false', but {}", bool_str);
  return equal(bool_str, "true"sv);
}

additional_stop parse_additional_stop(xml_node const additional_stop_xml) {
  additional_stop as{};

  auto element_xml = additional_stop_xml.child("infrastructureElement");
  if (static_cast<bool>(element_xml)) {
    uLOG(utl::warn) << "should parse infrastructure element for additional "
                       "stop, but this is not implemented yet";
  }

  as.duration_ =
      parse_duration(additional_stop_xml.child_value("additionalStopTime"));

  auto is_before_halt = additional_stop_xml.child("isBeforeArrivalDeparture");
  if (static_cast<bool>(is_before_halt)) {
    as.is_before_halt_ = parse_boolean(is_before_halt.child_value());
  }

  return as;
}

utls::result<stop_sequence> parse_sequence(xml_node const sequence_xml,
                                           infrastructure const& infra) {
  stop_sequence sequence;

  auto const sequence_points_xml = sequence_xml.child("sequenceServicePoints");
  for (auto const point_xml : sequence_points_xml.children()) {
    sequence_point st;

    std::string_view const ds100{point_xml.child_value("servicePoint")};
    std::string_view const route_name{point_xml.child_value("trackSystem")};

    auto station_it = infra->ds100_to_station_.find(ds100);
    if (station_it == std::end(infra->ds100_to_station_)) {
      return std::unexpected(error::kss::STATION_NOT_FOUND);
    }

    auto route_it = station_it->second->station_routes_.find(route_name);
    if (route_it != std::end(station_it->second->station_routes_)) {
      st.station_route_ = route_it->second->id_;
    } else {
      return std::unexpected(error::kss::STATION_ROUTE_NOT_FOUND);
    }

    if (auto const arr = point_xml.child("arrival"); arr) {
      st.arrival_ = soro::optional<relative_time>{parse_time_offset(arr)};
    }

    if (auto const dep = point_xml.child("departure"); dep) {
      st.departure_ = soro::optional<relative_time>{parse_time_offset(dep)};
    }

    auto const stop_mode_xml = point_xml.child("stopMode");
    if (static_cast<bool>(stop_mode_xml)) {
      st.type_ = KEY_TO_STOP_TYPE.at(
          *point_xml.child("stopMode").attribute("Schluessel").value());
    }

    auto const min_stop_time_xml = point_xml.child("minStopTime");
    if (static_cast<bool>(min_stop_time_xml)) {
      st.min_stop_time_ = parse_duration(min_stop_time_xml.child_value());
    } else {
      st.min_stop_time_ = duration2::zero();
    }

    utls::sasserts([&] {
      if (st.is_transit() && st.departure_.has_value()) {
        utls::sassert(route_it->second->runtime_checkpoint_.has_value(),
                      "found departure time value, but not valid runtime "
                      "checkpoint in station route in transit sequence point");
      }
    });

    // ignore additional stops for now
    /*
    auto additional_stop_xml = point_xml.child("additionalStop");
    if (static_cast<bool>(additional_stop_xml)) {
      st.additional_stop_ = parse_additional_stop(additional_stop_xml);
    }
     */

    sequence.points_.emplace_back(st);
  }

  sequence.break_in_ = equal(sequence_xml.child_value("breakin"), "true");
  sequence.break_out_ = equal(sequence_xml.child_value("breakout"), "true");

  return sequence;
}

utls::result<rs::train_physics> parse_characteristic(
    xml_node const charac_xml, rs::rolling_stock const& rolling_stock) {
  auto const traction_units_xml = charac_xml.child("tractionUnits");

  soro::vector<rs::traction_vehicle> tvs;

  for (auto const traction_unit_xml : traction_units_xml.children()) {
    auto const series_xml = traction_unit_xml.child("tractionUnitDesignSeries");

    std::string_view const series = series_xml.child_value("designSeries");
    std::string_view const owner =
        series_xml.child("designSeries").attribute("Nr").value();
    std::integral auto const variant =
        utls::parse_int<rs::variant_id>(series_xml.child_value("variante"));

    auto tv = rolling_stock.get_traction_vehicle(series, owner, variant);
    if (!tv) {
      return utls::propagate(tv);
    }

    tvs.emplace_back(*tv);
  }

  auto const length = si::from_m(
      utls::parse_fp<si::precision>(charac_xml.child_value("totalLength")));

  auto const carriage_weight = si::from_ton(
      utls::parse_fp<si::precision>(charac_xml.child_value("totalWeight")));

  auto const max_speed = si::from_km_h(
      utls::parse_fp<si::precision>(charac_xml.child_value("maxVelocity")));

  auto const freight = static_cast<rs::FreightTrain>(
      utls::equal(charac_xml.child_value("relevantStopPositionMode"), "G"));

  auto const ctc = static_cast<rs::CTC>(
      utls::equal(charac_xml.child_value("trainProtection"), "true"));

  return rs::train_physics{tvs, carriage_weight, length, max_speed,
                           ctc, freight};
}

utls::result<void> is_supported(stop_sequence const& stop_sequence,
                                infrastructure const& infra, train const& t) {

  if (t.has_ctc()) {
    return std::unexpected(error::kss::CTC_NOT_SUPPORTED);
  }

  if (stop_sequence.points_.size() < 2) {
    return std::unexpected(error::kss::SINGLE_STOP_NOT_SUPPORTED);
  }

  if (stop_sequence.break_in_) {
    return std::unexpected(error::kss::BREAK_IN_NOT_SUPPORTED);
  }

  if (stop_sequence.break_out_) {
    return std::unexpected(error::kss::BREAK_OUT_NOT_SUPPORTED);
  }

  if (!stop_sequence.points_.front().is_halt()) {
    return std::unexpected(error::kss::FIRST_STOP_NO_HALT_NOT_SUPPORTED);
  }

  if (!stop_sequence.points_.back().is_halt()) {
    return std::unexpected(error::kss::LAST_STOP_NO_HALT_NOT_SUPPORTED);
  }

  for (auto const& sequence_point : stop_sequence.points_) {
    auto const sr = infra->station_routes_[sequence_point.station_route_];

    if (sequence_point.is_halt() &&
        !sr->get_halt_idx(t.freight()).has_value()) {
      return std::unexpected(
          error::kss::STOP_IS_HALT_BUT_STATION_ROUTE_NO_HALT);
    }

    if (sequence_point.has_transit_time() &&
        !sr->get_runtime_checkpoint_node().has_value()) {
      return std::unexpected(
          error::kss::TRANSIT_TIME_BUT_NO_RUNTIME_CHECKPOINT);
    }
  }

  return {};
}

train::number parse_train_number(xml_node const train_number_xml) {
  train::number tn{};

  tn.main_ = utls::parse_int<train::number::main_t>(
      train_number_xml.child_value("mainNumber"));
  tn.sub_ = utls::parse_int<train::number::sub_t>(
      train_number_xml.child_value("subNumber"));

  return tn;
}

soro::map<element_id, sequence_point::ptr> get_interlocking_to_sequence_point(
    soro::vector<sequence_point> const& sequence_points,
    rs::FreightTrain const freight, infrastructure const& infra) {
  soro::map<element_id, sequence_point::ptr> halt_to_sequence_point;

  for (auto const& sp : sequence_points) {
    if (!sp.is_halt()) {
      continue;
    }

    auto const node = sp.get_node(freight, infra);

    utls::sassert(node.has_value(), "sequence point has no associated node");

    halt_to_sequence_point.emplace((*node)->element_->id(), &sp);
  }

  return halt_to_sequence_point;
};

utls::result<train> parse_construction_train(
    xml_node const construction_train_xml, infrastructure const& infra) {
  train t;

  t.number_ = parse_train_number(construction_train_xml.child("trainNumber"));

  auto const services_xml = construction_train_xml.child("services");
  t.service_days_ = parse_services(services_xml);

  auto const sequence_xml = construction_train_xml.child("sequence");
  auto const train_sequence = parse_sequence(sequence_xml, infra);

  if (!train_sequence) {
    return utls::propagate(train_sequence);
  }

  auto const characteristic_xml =
      construction_train_xml.child("characteristic");
  auto phys = parse_characteristic(characteristic_xml, infra->rolling_stock_);
  if (!phys) {
    return utls::propagate(phys);
  }
  t.physics_ = std::move(*phys);

  if (auto supported = is_supported(*train_sequence, infra, t); !supported) {
    return utls::propagate(supported);
  }

  t.break_in_ = train_sequence->break_in_;
  t.break_out_ = train_sequence->break_out_;

  auto transformed =
      transform_to_interlocking(*train_sequence, t.physics_.freight(), infra);

  if (!transformed) {
    return utls::propagate(transformed);
  }

  t.path_ = std::move(transformed->path_);
  t.sequence_points_ = std::move(transformed->sequence_points_);

  t.halt_to_sequence_point_ = get_interlocking_to_sequence_point(
      t.sequence_points_, t.freight(), infra);

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
  throw utl::fail("not reachable");
}

utls::result<station::ptr> ds100_to_station(std::string_view const ds100,
                                            infrastructure const& infra) {
  auto const station_it = infra->ds100_to_station_.find(ds100);
  if (station_it == std::end(infra->ds100_to_station_)) {
    return std::unexpected(error::kss::STATION_NOT_FOUND);
  }

  return station_it->second;
}

static std::map<connection::mode, size_t> mode_stats;

utls::result<connection> parse_connection(xml_node const connection_xml,
                                          infrastructure const& infra) {
  connection c;

  c.mode_ = parse_connection_mode(connection_xml.child("connectionMode"));

  mode_stats[c.mode_]++;

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
    c.time_ = soro::optional<duration2>{parse_duration(time_xml.child_value())};
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
    result.emplace_back(std::move(*connection));
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

  utls::sassert(first_train->physics_ == second_train->physics_,
                "trains have different physics");
  utls::sassert(first_train->path_.back() == second_train->path_.front(),
                "trains are not connected");
  utls::sassert(first_train->sequence_points_.back() ==
                    second_train->sequence_points_.front(),
                "trains are not connected");
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

utls::result<parsed_kss_train> parse_kss_train(xml_node const train_xml,
                                               infrastructure const& infra) {
  parsed_kss_train result;

  // don't parse worked on trains
  auto const train_status = train_xml.attribute("trainStatus").value();
  if (!utls::equal(train_status, "freig")) {
    return std::unexpected(error::kss::WORKED_ON_TRAIN);
  }

  auto const fine_construction_xml = train_xml.child("fineConstruction");

  for (auto const construction_train_xml :
       fine_construction_xml.children("constructionTrain")) {

    auto const train = parse_construction_train(construction_train_xml, infra);

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
    std::filesystem::path const& fp, infrastructure const& infra,
    error::stats& stats) {
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
    auto const parsed_kss_train = parse_kss_train(train_xml, infra);

    if (!parsed_kss_train) {
      stats.count(parsed_kss_train);
      continue;
    }

    utl::concat(result.trains_, parsed_kss_train->trains_);
    utl::concat(result.connections_, parsed_kss_train->connections_);
  }

  return result;
}

struct unique_key {
  CISTA_COMPARABLE()
  absolute_time time_;
  station_route::id station_route_;
};

struct sr_usage {
  CISTA_COMPARABLE();

  absolute_time start_;
  train::id train_;
};

void duplicate_finder(soro::vector<train> const& trains,
                      infrastructure const& infra) {
  utl::scoped_timer const timer("find duplicates");

  std::vector<std::vector<sr_usage>> sr_usages(infra->station_routes_.size());

  for (auto const& train : trains) {
    for (auto const midnight : train.departures()) {
      for (auto const& sp : train.sequence_points_) {

        if (auto const arr = sp.absolute_arrival(midnight); arr.has_value()) {
          sr_usages[sp.station_route_].emplace_back(sr_usage{*arr, train.id_});
        }

        //        if (auto const dep = sp.absolute_departure(midnight);
        //        dep.has_value()) {
        //          unique_key const key{.time_ = *dep,
        //                               .station_route_ = sp.station_route_};
        //          duplicates[key].emplace_back(train.id_);
        //        }
      }
    }
  }

  for (auto& v : sr_usages) {
    std::sort(std::begin(v), std::end(v));
  }

  //  auto const& t = trains.front();
  //  t.sequence_points_.front().

  //  auto const cmp = [](unique_key const& k1, unique_key const& k2) {
  //    return k1.time_ == k2.time_ ? k1.station_route_ < k2.station_route_
  //                                : k1.time_ < k2.time_;
  //    //
  //    //    if (k1.time_ < k2.time_) {
  //    //      return true;
  //    //    }
  //    //
  //    //    if (k1.time_ > k2.time_) {
  //    //      return false;
  //    //    }
  //    //
  //    //    return k1.station_route_ < k2.station_route_;
  //  };

  std::map<unique_key, soro::vector<train::id>> duplicates;

  for (auto const& train : trains) {
    for (auto const midnight : train.departures()) {

      for (auto const& sp : train.sequence_points_) {

        if (auto const arr = sp.absolute_arrival(midnight); arr.has_value()) {
          unique_key const key{.time_ = *arr,
                               .station_route_ = sp.station_route_};
          duplicates[key].emplace_back(train.id_);
        }

        if (auto const dep = sp.absolute_departure(midnight); dep.has_value()) {
          unique_key const key{.time_ = *dep,
                               .station_route_ = sp.station_route_};
          duplicates[key].emplace_back(train.id_);
        }
      }
    }
  }

  std::set<std::pair<train::id, train::id>> unique_duplicates;

  soro::size_t dup_count = 0;
  for (auto const& [_, dups] : duplicates) {
    if (dups.size() == 2 && dups[0] != dups[1]) {
      ++dup_count;
      unique_duplicates.emplace(dups[0], dups[1]);
    }
  }

  for (auto const& unique_dup : unique_duplicates) {
    std::cout << "unique: " << unique_dup.first << " " << unique_dup.second
              << "\n";
  }
  std::cout << "total dup pairs: " << dup_count << "\n";
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

  auto const first_fp = begin(fs::directory_iterator{opts.timetable_path_});
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
    result.start_ = std::min(result.start_, train.first_absolute_departure());
    result.end_ = std::max(result.end_, train.last_absolute_arrival());
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
    return conn.first_train_ == train::INVALID ||
           conn.second_train_ == train::INVALID;
  });

  uLOG(utl::info) << "resolved connections: " << connections.size();
}

utls::result<base_timetable> parse_kss_timetable(
    timetable_options const& opts, infra::infrastructure const& infra) {
  auto const required_version = get_required_infra_version(opts);

  if (required_version != infra->version_) {
    return utls::unexpected(error::kss::INFRASTRUCTURE_VERSION_MISMATCH);
  }

  utl::scoped_timer const timetable_timer("parsing kss timetable");

  base_timetable bt;
  bt.source_ = opts.timetable_path_.filename().string();

  error::stats stats("parsing kss timetable");

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

  utl::parallel_for_run(work_todo.size(), [&](auto&& work_id) {
    work_todo[work_id].result_ =
        parse_timetable_file(work_todo[work_id].timetable_file_, infra, stats);
  });

  for (auto const& work_item : work_todo) {
    utl::concat(bt.trains_, work_item.result_.trains_);
    utl::concat(bt.connections_, work_item.result_.connections_);
  }

  set_ids(bt.trains_);
  //  resolve_connections(bt.connections_, bt.trains_, stats);

  uLOG(utl::info) << "total trains successfully parsed: " << bt.trains_.size();
  stats.report();

  //  duplicate_finder(bt.trains_);
  print_mode_stats(mode_stats);

  bt.interval_ = get_interval(bt.trains_);

  return bt;
}

}  // namespace soro::tt
