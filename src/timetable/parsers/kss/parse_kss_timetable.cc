#include "soro/timetable/parsers/kss/parse_kss_timetable.h"

#include <charconv>
#include <set>

#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/timetable/bitfield.h"
#include "soro/timetable/parsers/raw_to_trains.h"
#include "soro/timetable/parsers/station_route_to_interlocking_route.h"

#include "pugixml.hpp"

namespace soro::tt {

using namespace pugi;
using namespace soro::utls;
using namespace soro::infra;

namespace fs = std::filesystem;

bool is_kss_timetable(timetable_options const& opts) {
  if (!std::filesystem::is_directory(opts.timetable_path_)) {
    return false;
  }

  return utls::all_of(fs::directory_iterator{opts.timetable_path_},
                      [](auto&& dir_entry) {
                        auto const& fname = dir_entry.path().filename();
                        return fname.extension() == ".xml" &&
                               fname.string().starts_with("KSS-");
                      });
}

struct train_number {
  uint32_t main_;
  uint32_t sub_;
};

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

        utls::sassert(bitmask_length == distance(start_date, end_date) + 1,
                      "Bitmask has not the same size as distance between start "
                      "and end date!");

        total_service_days +=
            std::count(bitmask, bitmask + bitmask_length, '1');
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

  std::integral auto const hours = parse_int<uint64_t>(time_str, time_str + 2);
  std::integral auto const minutes =
      parse_int<uint64_t>(time_str + 3, time_str + 5);
  std::integral auto const seconds =
      parse_int<uint64_t>(time_str + 6, time_str + 8);

  uint64_t result = (hours * 60 * 60) + (minutes * 60) + seconds;

  if (static_cast<bool>(time_xml.child("nightLeap"))) {
    result += 24 * 60 * 60;
  }

  return relative_time{result};
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

  uint64_t dur = 0UL;

  while (letter_it != str_end) {
    auto next_cap = std::find_if(letter_it, str_end, [](char const c) {
      return (std::isalpha(c) != 0) && (std::isupper(c) != 0);
    });

    switch (*next_cap) {
      case 'H': {
        std::integral auto const val = parse_int<uint64_t>(letter_it, next_cap);
        dur += val * 60 * 60;
        break;
      }
      case 'M': {
        std::integral auto const val = parse_int<uint64_t>(letter_it, next_cap);
        dur += val * 60;
        break;
      }
      case 'S': {
        auto val = utls::parse_fp<double>(letter_it, next_cap);
        dur += static_cast<uint64_t>(std::round(val));
        break;
      }
    }

    letter_it = next_cap + 1;
  }

  return duration2{dur};
}

raw_train::run parse_sequence(xml_node const sequence_xml,
                              infrastructure const& infra) {
  raw_train::run run;

  auto const sequence_points_xml = sequence_xml.child("sequenceServicePoints");
  for (auto const point_xml : sequence_points_xml.children()) {
    std::string_view const ds100{point_xml.child_value("servicePoint")};
    std::string_view const route_name{point_xml.child_value("trackSystem")};

    auto station_it = infra->ds100_to_station_.find(ds100);
    if (station_it == std::end(infra->ds100_to_station_)) {
      uLOG(utl::warn) << "Could not find station " << ds100 << ".";
      return {};
    }

    auto route_it = station_it->second->station_routes_.find(route_name);
    if (route_it != std::end(station_it->second->station_routes_)) {
      run.routes_.push_back(route_it->second);
    } else {
      //      uLOG(utl::warn) << "Could not find station route " << route_name
      //      << " in "
      //                      << ds100 << ".";
      return {};
    }

    stop st;

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
    }

    run.stops_.push_back(st);
  }

  run.break_in_ = equal(sequence_xml.child_value("breakin"), "true");
  run.break_out_ = equal(sequence_xml.child_value("breakout"), "true");

  return run;
}

raw_train::characteristic parse_characteristic(xml_node const charac_xml) {
  raw_train::characteristic c;

  auto const traction_units_xml = charac_xml.child("tractionUnits");

  for (auto const traction_unit_xml : traction_units_xml.children()) {
    auto const series_xml = traction_unit_xml.child("tractionUnitDesignSeries");

    c.traction_units_.push_back(
        {.series_ = series_xml.child_value("designSeries"),
         .owner_ = series_xml.attribute("Nr").value(),
         .variant_ = utls::parse_int<rs::variant_id>(
             series_xml.child_value("variante"))});
  }

  c.length_ = si::from_m(
      utls::parse_fp<si::precision>(charac_xml.child_value("totalLength")));

  c.carriage_weight_ = si::from_ton(
      utls::parse_fp<si::precision>(charac_xml.child_value("totalWeight")));

  c.max_speed_ = si::from_km_h(
      utls::parse_fp<si::precision>(charac_xml.child_value("maxVelocity")));

  c.freight_ = static_cast<rs::FreightTrain>(
      utls::equal(charac_xml.child_value("relevantStopPositionMode"), "G"));

  return c;
}

enum class failure_reason : uint8_t {
  worked_on_train,
  failed_getting_station_route_path,
  failed_getting_interlocking_route_path,
  first_stop_is_no_halt_and_train_is_not_breaking_in,
  first_stop_is_halt_and_first_sr_has_no_halt
};

static std::map<failure_reason, std::size_t> failures = {  // NOLINT
    {failure_reason::worked_on_train, 0},
    {failure_reason::failed_getting_station_route_path, 0},
    {failure_reason::failed_getting_interlocking_route_path, 0},
    {failure_reason::first_stop_is_no_halt_and_train_is_not_breaking_in, 0},
    {failure_reason::first_stop_is_halt_and_first_sr_has_no_halt, 0}

};

static std::map<failure_reason, std::string> failure_text = {  // NOLINT
    {failure_reason::worked_on_train, "Skipped worked on trains: "},
    {failure_reason::failed_getting_station_route_path,
     "Failed getting station route path for trains: "},
    {failure_reason::failed_getting_interlocking_route_path,
     "Failed generating interlocking route path: "},
    {failure_reason::first_stop_is_no_halt_and_train_is_not_breaking_in,
     "First stop is not a halt, but train is not breaking in: "},
    {failure_reason::first_stop_is_halt_and_first_sr_has_no_halt,
     "First stop is halt, but first station route has no halt: "}

};

void print_failures() {
  for (auto const& [reason, count] : failures) {
    uLOG(utl::warn) << failure_text[reason] << count;
  }
}

bool ignore_for_now(train::ptr const train, infrastructure const& infra) {

  if (!train->stops.front().is_halt() && !train->break_in_) {
    ++failures
        [failure_reason::first_stop_is_no_halt_and_train_is_not_breaking_in];
    return true;
  }

  auto const first_halt =
      train->first_station_route(infra)->get_halt_node(train->freight_);
  if (train->stops.front().is_halt() && !first_halt.has_value()) {
    ++failures[failure_reason::first_stop_is_halt_and_first_sr_has_no_halt];
    return true;
  }

  return false;
}

base_timetable parse_kss_timetable(timetable_options const& opts,
                                   infra::infrastructure const& infra) {
  utl::scoped_timer const timetable_timer("Parsing Timetable");

  base_timetable bt;

  std::size_t total_trains = 0;
  std::size_t total_train_runs = 0;

  for (auto const& dir_entry : fs::directory_iterator{opts.timetable_path_}) {
    auto const loaded_file = utls::load_file(dir_entry.path());

    xml_document file_xml;
    auto success = file_xml.load_buffer(
        reinterpret_cast<void const*>(loaded_file.contents_.data()),
        loaded_file.contents_.size());

    utl::verify(success,
                "PugiXML error '{}' while parsing {} from KSS timetable {}!",
                success.description(), loaded_file.path_, opts.timetable_path_);

    auto const railml_xml = file_xml.child("KSS").child("railml");
    auto const timetable_xml = railml_xml.child("timetable");

    for (auto const train_xml : timetable_xml.children("train")) {
      ++total_trains;

      // don't parse worked on trains
      auto const train_status = train_xml.attribute("trainStatus").value();
      if (!utls::equal(train_status, "freig")) {
        ++failures[failure_reason::worked_on_train];
        continue;
      }

      for (auto const construction_train_xml :
           train_xml.child("fineConstruction").children("constructionTrain")) {
        ++total_train_runs;

        auto t = soro::make_unique<train>();

        auto const sequence_xml = construction_train_xml.child("sequence");
        auto const run = parse_sequence(sequence_xml, infra);

        if (run.routes_.empty()) {
          ++failures[failure_reason::failed_getting_station_route_path];
          continue;
        }

        auto const services_xml = construction_train_xml.child("services");
        t->bitfield_ = parse_services(services_xml);

        auto const characteristic_xml =
            construction_train_xml.child("characteristic");
        auto const charac = parse_characteristic(characteristic_xml);

        t->path_ = get_interlocking_route_path(run, charac.freight_, infra);

        if (t->path_.empty()) {
          ++failures[failure_reason::failed_getting_interlocking_route_path];
          continue;
        }

        if (ignore_for_now(t.get(), infra)) {
          continue;
        }

        bt.train_store_.emplace_back();
        bt.train_store_.back() = std::move(t);
      }
    }
  }

  uLOG(utl::info) << "Total trains in data: " << total_trains;
  uLOG(utl::info) << "Total train runs in data: " << total_train_runs;
  uLOG(utl::info) << "Total trains runs successfully parsed: "
                  << bt.train_store_.size();

  print_failures();
  print_ir_generating_failures();

  return bt;
}

}  // namespace soro::tt
