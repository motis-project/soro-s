#include "soro/timetable/parsers/kss/parse_kss_timetable.h"

#include <charconv>
#include <set>

#include "utl/logging.h"

#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/timetable/bitfield.h"
#include "soro/timetable/parsers/raw_to_trains.h"

#include "pugixml.hpp"

namespace soro::tt {

using namespace pugi;
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

template <std::integral T>
T parse_int(const char* const start, const char* const end) {
  T val = std::numeric_limits<T>::max();

  auto const [ptr, ec] = std::from_chars(start, end, val);

  utls::sassert(ec == std::errc{}, "Error while parsing integer input {}.",
                std::quoted(start));

  utls::sassert(ptr == end, "Error while parsing integer input {}.",
                std::quoted(start));

  return val;
}

template <std::integral T>
T parse_int(const char* const c) {
  return parse_int<T>(c, c + strlen(c));
}

template <std::integral T>
T parse_int(std::string const& s) {
  return parse_int<T>(s.data(), s.data() + s.size());
}

template <std::integral T>
T parse_int(std::string_view const& s) {
  return parse_int<T>(s.data(), s.data() + s.size());
}

struct train_number {
  uint32_t main_;
  uint32_t sub_;
};

// struct service {
//   date::year_month_day start_;
//   date::year_month_day end_;
//   bitfield bitfield_;
// };

bitfield parse_services(xml_node const services_xml) {

  auto const parse_date = [](std::string const& s) {
    auto const split = utls::split(s, "-");

    utls::sassert(split.size() == 3,
                  "Expected a date to be composed out of three parts, got {}.",
                  split.size());

    date::year y{parse_int<int>(split[0])};
    date::month m{parse_int<unsigned>(split[1])};
    date::day d{parse_int<unsigned>(split[2])};

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
      bf.set(date, true);
    } else if (utls::equal(special_xml.attribute("type").value(), "exclude")) {
      bf.set(date, false);
    } else {
      uLOG(utl::warn) << "Found " << special_xml.attribute("type").value()
                      << " special service type but case is not implemented";
    }
  };

  bitfield accumulated_bitfield;

  for (auto const service_xml : services_xml.children("service")) {
    if (auto bitmask_xml = service_xml.attribute("bitMask"); bitmask_xml) {
      bitfield bf(parse_date(service_xml.attribute("startDate").value()),
                  parse_date(service_xml.attribute("endDate").value()),
                  service_xml.attribute("bitMask").value());

      accumulated_bitfield.ok() ? accumulated_bitfield |= bf
                                : accumulated_bitfield = bf;
    } else {
      utls::sassert(
          accumulated_bitfield.ok(),
          "Trying to set special date, but base bitfield is not constructed");
      parse_special_service(accumulated_bitfield, service_xml);
    }
  }

  return accumulated_bitfield;
}

stop::time_offset parse_time_offset(xml_node const time_xml) {
  auto const time_str = time_xml.child_value("time");

  utls::sassert(strlen(time_str) == 8);

  std::integral auto const hours =
      parse_int<stop::time_offset>(time_str, time_str + 1);
  std::integral auto const minutes =
      parse_int<stop::time_offset>(time_str + 2, time_str + 3);
  std::integral auto const seconds =
      parse_int<stop::time_offset>(time_str + 4, time_str + 5);

  auto result = (hours * 60 * 60) + (minutes * 60) + seconds;

  if (static_cast<bool>(time_xml.child("nightLeap"))) {
    result += 24 * 60 * 60;
  }

  return result;
};

utls::duration parse_duration(char const* const dur_str) {
  utls::sassert(dur_str[0] == 'P');
  utls::sassert(dur_str[1] == 'T');

  // Example duration strings:
  // PT_
  // PT6S_
  // PT5M_
  // PT5M6S_
  // PT5M54S_
  // PT15M54S_

  auto const str_end = dur_str + strlen(dur_str);

  auto letter_it = dur_str + 2;

  auto dur = utls::duration::ZERO;

  while (letter_it != str_end) {
    auto next_cap = std::find_if(letter_it, str_end, [](char const c) {
      return (std::isalpha(c) != 0) && (std::isupper(c) != 0);
    });

    utls::sassert(
        std::find_if(letter_it, next_cap,
                     [](auto const c) { return std::ispunct(c); }) == next_cap,
        "Found punctuation character in seconds string '{}', feature is not "
        "implemented",
        std::string_view(letter_it, next_cap));

    std::integral auto const val = parse_int<uint32_t>(letter_it, next_cap);

    switch (*next_cap) {
      case 'H': {
        dur += utls::from_hours(val);
        break;
      }
      case 'M': {
        dur += utls::from_minutes(val);
        break;
      }
      case 'S': {
        dur += utls::from_seconds(val);
        break;
      }
    }

    letter_it = next_cap + 1;
  }

  return dur;
};

raw_train::run parse_sequence(xml_node const sequence_xml,
                                 infrastructure const& infra) {
  raw_train::run run;

  auto const sequence_points_xml = sequence_xml.child("sequenceServicePoints");
  for (auto const point_xml : sequence_points_xml.children()) {
    auto const ds100 = point_xml.child_value("servicePoint");
    auto const station_route = point_xml.child_value("trackSystem");

    auto const station = infra->ds100_to_station_.at(ds100);
    auto const route = station->station_routes_.at(station_route);
    run.routes_.push_back(route);

    stop st;

    st.arrival_ = parse_time_offset(point_xml.child("arrival"));
    st.departure_ = parse_time_offset(point_xml.child("departure"));

    auto const stop_mode_xml = point_xml.child("stopMode");
    if (static_cast<bool>(stop_mode_xml)) {
      st.type_ = KEY_TO_STOP_TYPE.at(
          *point_xml.child("stopMode").attribute("Schluessel").value());
    }

    auto const min_stop_time_xml = point_xml.child("minStopTime");
    if (static_cast<bool>(min_stop_time_xml)) {
      st.min_stop_time_ = parse_duration(min_stop_time_xml.value());
    }

    run.stops_.push_back(st);
  }

  return run;
}

base_timetable parse_kss_timetable(timetable_options const& opts,
                                   infra::infrastructure const& infra) {
  base_timetable bt;

  std::set<std::string> status_map;

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
      if (!utls::equal(train_xml.attribute("trainStatus").value(), "freig")) {
        status_map.insert(train_xml.attribute("trainStatus").value());
        continue;
      }

      std::integral auto const id =
          parse_int<train::id>(train_xml.attribute("trainID").value());
      std::cout << "id: " << id << '\n';

      //    auto const entries_xml = train_xml.child("timetableentries");
      auto const fine_construction_xml = train_xml.child("fineConstruction");
      auto const construction_train_xml =
          fine_construction_xml.child("constructionTrain");

      auto const services_xml = construction_train_xml.child("services");
      auto const bf = parse_services(services_xml);
      std::cout << "bf: " << bf.first_date_ << '\n';
      std::cout << "bf: " << bf.last_date_ << '\n';

      //    auto const characteristics_xml =
      //        construction_train_xml.child("characteristics");
      auto const sequence_xml = construction_train_xml.child("sequence");
      auto const run = parse_sequence(sequence_xml, infra);
    }

    std::cout << "status map\n";
    for (auto const& e : status_map) {
      std::cout << e << '\n';
    }

    return bt;
  }

  return bt;
}

}  // namespace soro::tt
