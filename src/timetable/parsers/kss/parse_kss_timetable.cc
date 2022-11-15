#include "soro/timetable/parsers/kss/parse_kss_timetable.h"

#include <set>

#include "utl/logging.h"

#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/timetable/bitfield.h"

#include "pugixml.hpp"

namespace soro::tt {

using namespace pugi;

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
T parse_int(const char* const c) {
  if constexpr (std::is_unsigned_v<T>) {
    return static_cast<T>(std::stoull(c));
  } else {
    return static_cast<T>(std::stoll(c));
  }
}

template <std::integral T>
T parse_int(std::string const& s) {
  return parse_int<T>(s.data());
}

template <std::integral T>
T parse_int(std::string_view const& s) {
  return parse_int<T>(s.data());
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

  bitfield bf;

  for (auto const service_xml : services_xml.children("service")) {
    if (auto bitmask_xml = service_xml.attribute("bitMask"); bitmask_xml) {
      bf = bitfield(parse_date(service_xml.attribute("startDate").value()),
                    parse_date(service_xml.attribute("endDate").value()),
                    service_xml.attribute("bitMask").value());
    } else {
      utls::sassert(
          bf.first_date_.ok() && bf.last_date_.ok(),
          "Trying to set special date, but base bitfield is not constructed");
      parse_special_service(bf, service_xml);
    }
  }

  return bf;
}

base_timetable parse_kss_timetable(timetable_options const& opts,
                                   infra::infrastructure const&) {
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

      auto const id =
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
      //    auto const sequence_xml = construction_train_xml.child("sequence");
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
