#include "soro/infrastructure/parsers/iss/get_infra_stats.h"

#include "pugixml.hpp"

#include "utl/verify.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

#include "soro/utls/string.h"

namespace soro::infra {

void count_elements_in_section(pugi::xml_node xml_section, infra_stats& is) {
  ++is.sections_;

  for (auto const xml_element : xml_section.child(RAIL_PLAN_NODE).children()) {
    auto const t = get_type(xml_element.name());
    if (t != type::INVALID) {
      ++(is.number(t));
    }
  }
}

void count_elements_in_station(pugi::xml_node xml_station, infra_stats& is) {
  ++is.stations_;

  for (auto const& xml_section :
       xml_station.child(RAIL_PLAN_SECTIONS).children(RAIL_PLAN_SECTION)) {
    count_elements_in_section(xml_section, is);
  }
}

void count_elements_in_file(utls::loaded_file const& xml_file,
                            infra_stats& is) {
  pugi::xml_document d;
  auto success = d.load_buffer(reinterpret_cast<void const*>(xml_file.begin()),
                               xml_file.size());
  utl::verify(success, "bad xml: {}", success.description());

  for (auto const& xml_station : d.child(XML_ISS_DATA)
                                     .child(RAIL_PLAN_STATIONS)
                                     .children(RAIL_PLAN_STATION)) {
    count_elements_in_station(xml_station, is);
  }
}

infra_stats get_infra_stats(iss_files const& files) {
  infra_stats is;

  for (auto const& file : files.rail_plan_files_) {
    count_elements_in_file(file, is);
  }

  // for every border pair there is an extra (empty) section
  is.sections_ += is.number(type::BORDER) / 2;

  for (auto t : all_types()) {
    // simple elements appear two times in the infrastructure data,
    // but are modelled as one element in the graph
    // exception: borders
    // TODO(julian) maybe remove half of all border elements?
    if (is_simple_element(t) && t != type::BORDER) {
      is.number(t) /= 2;
    }

    // every switch is represented three times in the infrastructure data
    if (t == type::SIMPLE_SWITCH) {
      is.number(t) /= 3;
    }

    if (t == type::CROSS) {
      is.number(t) /= 4;
    }
  }

  return is;
}

}  // namespace soro::infra