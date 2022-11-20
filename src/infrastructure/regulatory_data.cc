#include "soro/infrastructure/regulatory_data.h"

#include "pugixml.hpp"

#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

regulatory_station_data parse_regulatory_stations(
    std::vector<utls::loaded_file> const& regulatory_station_files) {
  utl::scoped_timer const regulartory_timer("Parsing Regulatory Data");

  regulatory_station_data station_data;

  for (auto const& reg_file : regulatory_station_files) {
    uLOG(utl::info) << "Parsing " << reg_file.path_;

    pugi::xml_document d;
    auto success =
        d.load_buffer(reinterpret_cast<void const*>(reg_file.contents_.data()),
                      reg_file.contents_.size());
    utl::verify(success, "bad xml: {}", success.description());

    for (auto const& regulatory_station : d.child(XML_ISS_DATA)
                                              .child("Ordnungsrahmen")
                                              .child("Betriebsstellen")) {
      soro::string const ds100 = regulatory_station.child_value("DS100");
      soro::string const name = regulatory_station.child_value("Name");
      station_data.ds100_to_full_name_[ds100] = name;
    }
  }

  return station_data;
}

regulatory_line_data parse_regulatory_line_data(
    std::vector<utls::loaded_file> const&) {
  regulatory_line_data line_data;
  return line_data;
}

}  // namespace soro::infra