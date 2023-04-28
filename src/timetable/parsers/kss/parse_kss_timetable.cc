#include "soro/timetable/parsers/kss/parse_kss_timetable.h"

#include "pugixml.hpp"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/logging.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/statistics.h"
#include "soro/utls/string.h"

#include "soro/base/error.h"
#include "soro/timetable/bitfield.h"
#include "soro/timetable/parsers/kss/kss_error.h"
#include "soro/timetable/parsers/station_route_to_interlocking_route.h"

namespace soro::tt {

using namespace pugi;
using namespace soro::utls;
using namespace soro::infra;

namespace fs = std::filesystem;

bool is_kss_timetable(timetable_options const& opts) {
  if (!std::filesystem::is_directory(opts.timetable_path_)) {
    return false;
  }

  return utls::all_of(
      fs::directory_iterator{opts.timetable_path_}, [](auto&& dir_entry) {
        auto const& fn = dir_entry.path().filename();
        return !fs::is_directory(dir_entry) && fn.extension() == ".xml" &&
               (fn.string().starts_with("KSS-") ||
                fn.string().starts_with("."));
      });
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

    if (auto arr = point_xml.child("arrival"); arr) {
      st.arrival_ = parse_time_offset(arr);
    }

    if (auto dep = point_xml.child("departure"); dep) {
      st.departure_ = parse_time_offset(dep);
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

    if (st.is_transit() && valid(st.departure_)) {
      utls::sassert(route_it->second->runtime_checkpoint_.has_value(),
                    "Found departure time value, but not valid runtime "
                    "checkpoint in station route");
    }

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

  return t;
}

utls::result<soro::vector<train>> parse_kss_train(xml_node const train_xml,
                                                  infrastructure const& infra) {
  soro::vector<train> result;

  // don't parse worked on trains
  auto const train_status = train_xml.attribute("trainStatus").value();
  if (!utls::equal(train_status, "freig")) {
    return std::unexpected(error::kss::WORKED_ON_TRAIN);
  }

  for (auto const construction_train_xml :
       train_xml.child("fineConstruction").children("constructionTrain")) {

    auto const train = parse_construction_train(construction_train_xml, infra);

    if (!train) {
      return utls::propagate(train);
    }

    result.emplace_back(*train);
  }

  return result;
}

utls::result<soro::vector<train>> parse_timetable_file(
    std::filesystem::path const& fp, infrastructure const& infra,
    error::stats& stats) {
  soro::vector<train> result;

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
    auto const trains = parse_kss_train(train_xml, infra);

    if (!trains) {
      stats.count(trains);
      continue;
    }

    utl::concat(result, *trains);
  }

  return result;
}

void set_ids(soro::vector<train>& trains) {
  for (auto [id, train] : utl::enumerate(trains)) {
    train.id_ = static_cast<train::id>(id);
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

utls::result<base_timetable> parse_kss_timetable(
    timetable_options const& opts, infra::infrastructure const& infra) {
  auto const required_version = get_required_infra_version(opts);

  if (required_version != infra->version_) {
    return std::unexpected(error::kss::INFRASTRUCTURE_VERSION_MISMATCH);
  }

  utl::scoped_timer const timetable_timer("parsing kss timetable");
  error::stats stats("parsing kss timetable");

  base_timetable bt;
  bt.source_ = opts.timetable_path_.filename().string();

  struct work_item {
    fs::path timetable_file_;
    utls::result<soro::vector<train>> result_;
  };

  std::vector<work_item> work_todo;

  for (auto const& dir_entry : fs::directory_iterator{opts.timetable_path_}) {
    work_todo.emplace_back(work_item{dir_entry.path(), {}});
  }

  utl::parallel_for_run(work_todo.size(), [&](auto&& work_id) {
    work_todo[work_id].result_ =
        parse_timetable_file(work_todo[work_id].timetable_file_, infra, stats);
  });

  for (auto const& work_item : work_todo) {
    if (!work_item.result_) {
      return utls::propagate(work_item.result_);
    }

    utl::concat(bt.trains_, *work_item.result_);
  }

  set_ids(bt.trains_);

  bt.interval_ = get_interval(bt.trains_);

  uLOG(utl::info) << "total trains runs successfully parsed: "
                  << bt.trains_.size();
  stats.report();

  return bt;
}

}  // namespace soro::tt
