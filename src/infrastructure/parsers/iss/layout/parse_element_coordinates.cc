#include "soro/infrastructure/parsers/iss/layout/parse_element_coordinates.h"

#include <cmath>
#include <algorithm>
#include <iterator>

#include "pugixml.hpp"

#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/any_of.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/layout.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/layout/parse_coordinates.h"
#include "soro/infrastructure/parsers/iss/parse_helpers.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

soro::vector_map<element::id, coordinates> parse_raw_element_coordinates(
    iss_files const& iss_files, id_mapping const& to_element_id,
    soro::size_t const element_count) {
  utl::scoped_timer const timer("parsing raw element coordinates");

  soro::vector_map<element::id, coordinates> result(element_count);

  for (auto const& rail_plan_file_xml : iss_files.rail_plan_files_) {
    for (auto const& rp_station : rail_plan_file_xml.child(XML_ISS_DATA)
                                      .child(RAIL_PLAN_STATIONS)
                                      .children(RAIL_PLAN_STATION)) {
      for (auto const& rp_section :
           rp_station.child(RAIL_PLAN_SECTIONS).children(RAIL_PLAN_SECTION)) {

        for (auto const& rp_node :
             rp_section.child(RAIL_PLAN_NODE).children()) {
          if (auto const& coord_child = rp_node.child(PICTURE_COORDINATES);
              static_cast<bool>(coord_child)) {
            auto const rp_id = parse_rp_node_id(rp_node);
            auto const coords = parse_coordinates(coord_child);

            utls::sassert(to_element_id.first(rp_id) != element::invalid(),
                          "first must be valid");
            result[to_element_id.first(rp_id)] = coords;

            if (to_element_id.second(rp_id) != element::invalid()) {
              result[to_element_id.second(rp_id)] =
                  parse_coordinates(coord_child);
            }
          }
        }
      }
    }
  }

  utls::sassert(utls::any_of(result, [](auto&& e) { return e.valid(); }),
                "at least one element must have a valid coordinate");

  return result;
}

void add_track_element_coords(
    soro::vector_map<element::id, coordinates>& element_coordinates,
    sections const& sections) {
  utl::scoped_timer const timer("calculating track element coordinates");

  auto const get_next_with_coordinates = [](auto&& section, auto&& it) {
    auto const is_picture_point = [](auto&& e) {
      return e->is(type::PICTURE_POINT);
    };

    auto const pp_it =
        std::find_if(it, std::end(section.rising_order_), is_picture_point);

    return pp_it != std::end(section.rising_order_)
               ? pp_it
               : std::end(section.rising_order_) - 1;
  };

  for (auto const& section : sections) {
    auto from = std::begin(section.rising_order_);
    auto to = get_next_with_coordinates(section, from + 1);

    while (from != std::end(section.rising_order_) - 1) {
      auto start = element_coordinates[(*from)->get_id()];
      auto end = element_coordinates[(*to)->get_id()];

      utls::sassert(start.valid(), "start must be valid");
      utls::sassert(end.valid(), "end must be valid");

      auto const sloped_upward = start.y_ > end.y_;
      auto const left_to_right = start.x_ < end.x_;

      auto const x_total = std::abs(start.x_ - end.x_);
      auto const y_total = std::abs(start.y_ - end.y_);

      auto const total_elements =
          static_cast<double>(std::distance(from, to) + 1);

      soro::size_t idx = 1;
      for (auto curr = from + 1; curr != to; ++curr) {
        auto const fp_idx = static_cast<coordinates::precision>(idx);
        auto const x_distance = x_total * (fp_idx / (total_elements - 1.0));
        auto const y_distance = y_total * (fp_idx / (total_elements - 1.0));

        element_coordinates[(*curr)->get_id()].x_ =
            start.x_ + (left_to_right ? x_distance : -x_distance);
        element_coordinates[(*curr)->get_id()].y_ =
            start.y_ + (sloped_upward ? -y_distance : y_distance);

        ++idx;
      }

      from = to;
      to = get_next_with_coordinates(section, from + 1);
    }
  }
}

void adjust_station_coordinates(
    soro::vector_map<element::id, coordinates>& element_coordinates,
    soro::vector_map<station::id, station::ptr> const& stations) {
  utl::scoped_timer const timer("adjusting station coordinates.");

  for (auto const& station : stations) {
    auto min = coordinates::max();

    for (auto const& e : station->elements_) {
      auto const& e_coords = element_coordinates[e->get_id()];

      min.x_ = std::min(min.x_, e_coords.x_);
      min.y_ = std::min(min.y_, e_coords.y_);
    }

    // adjust the coordinates to the upper left
    for (auto const& e : station->elements_) {
      element_coordinates[e->get_id()] -= min;
    }
  }
}

soro::vector_map<element::id, coordinates> parse_element_coordinates(
    iss_files const& iss_files, infrastructure_t const& infra,
    construction_materials const& mats) {
  utl::scoped_timer const timer("creating element coordinates");

  auto element_coordinates = parse_raw_element_coordinates(
      iss_files, mats.to_element_id_, infra.graph_.elements_.size());
  add_track_element_coords(element_coordinates, infra.graph_.sections_);
  adjust_station_coordinates(element_coordinates, infra.stations_);

  // every element has a coordinate assigned
  utls::ensure(element_coordinates.size() == infra.graph_.elements_.size());
  utls::ensure(
      utls::all_of(element_coordinates, [](auto&& e) { return e.valid(); }));

  return element_coordinates;
}

}  // namespace soro::infra
