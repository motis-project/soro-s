#include "soro/infrastructure/layout/layout.h"

#include "pugixml.hpp"

#include "utl/enumerate.h"
#include "utl/logging.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/infrastructure_t.h"

namespace soro::layout {

using namespace utl;

using namespace soro::infra;
using namespace soro::utls;

coordinates parse_coordinates(pugi::xml_node const& coord_child) {
  return {.x_ = parse_fp<coordinates::precision, replace_comma::ON>(
              coord_child.child_value(X)),
          .y_ = parse_fp<coordinates::precision, replace_comma::ON>(
              coord_child.child_value(Y))};
}

void calculate_track_element_coords(layout& layout,
                                    soro::vector<section> const& sections) {
  uLOG(info) << "[ Layout ] Calculating track element coordinates.";

  for (auto const& section : sections) {
    auto const& start_coords = layout[section.elements_.front()->id()];
    auto const& end_coords = layout[section.elements_.back()->id()];

    auto const sloped_upward = start_coords.y_ > end_coords.y_;
    auto const left_to_right = start_coords.x_ < end_coords.x_;

    auto const x_total = std::abs(start_coords.x_ - end_coords.x_);
    auto const y_total = std::abs(start_coords.y_ - end_coords.y_);

    auto const total_elements =
        static_cast<coordinates::precision>(section.elements_.size());

    for (auto const [idx, element] : utl::enumerate(section.elements_)) {
      if (!element->is_track_element()) {
        continue;
      }

      auto const fp_idx = static_cast<coordinates::precision>(idx);
      auto const x_distance = x_total * (fp_idx / (total_elements + 1));
      auto const y_distance = y_total * (fp_idx / (total_elements + 1));

      layout[element->id()].x_ =
          start_coords.x_ + (left_to_right ? x_distance : -x_distance);
      layout[element->id()].y_ =
          start_coords.y_ + (sloped_upward ? -y_distance : y_distance);
    }
  }
}

// calculates track coordinates for split falling and rising paths
[[maybe_unused]] void calculate_track_element_coords_split(
    layout& layout, soro::vector<section> const& sections) {
  uLOG(info) << "[ Layout ] Calculating track element coordinates.";

  for (auto const& section : sections) {
    auto const& start_coords = layout[section.elements_.front()->id()];
    auto const& end_coords = layout[section.elements_.back()->id()];

    auto const sloped_upward = start_coords.y_ > end_coords.y_;
    auto const left_to_right = start_coords.x_ < end_coords.x_;

    auto const x_total = std::abs(start_coords.x_ - end_coords.x_);
    auto const y_total = std::abs(start_coords.y_ - end_coords.y_);

    // count rising and falling occurrences in a section
    // to correctly determine the coord data for track elements
    auto rising = 0.0F;
    auto falling = 0.0F;
    for (auto const& element : section.elements_) {
      if (element->is_track_element()) {
        element->as<track_element>().rising_ ? ++rising : ++falling;
      }
    }

    // now calculate coord data
    auto current_falling = 1.0F;
    auto current_rising = 1.0F;

    for (auto const& element : section.elements_) {
      if (element->is_track_element()) {
        if (element->as<track_element>().rising_) {
          auto const x_distance = x_total * (current_rising / (rising + 1));
          auto const y_distance = y_total * (current_rising / (rising + 1));

          layout[element->id()].x_ =
              start_coords.x_ + (left_to_right ? x_distance : -x_distance);
          layout[element->id()].y_ =
              start_coords.y_ + (sloped_upward ? -y_distance : y_distance);
          ++current_rising;
        } else {
          auto const x_distance =
              x_total * ((falling - current_falling + 1) / (falling + 1));
          auto const y_distance =
              y_total * ((falling - current_falling + 1) / (falling + 1));

          layout[element->id()].x_ =
              end_coords.x_ + (left_to_right ? -x_distance : x_distance);
          layout[element->id()].y_ =
              end_coords.y_ + (sloped_upward ? y_distance : -y_distance);
          ++current_falling;
        }
      }
    }
  }
}

void adjust_coordinates(layout& layout,
                        soro::vector<station::ptr> const& stations) {
  uLOG(info) << "[ Layout ] Adjusting coordinates.";

  for (auto const& station : stations) {
    coordinates::precision min_x{
        std::numeric_limits<coordinates::precision>::max()};
    coordinates::precision min_y{
        std::numeric_limits<coordinates::precision>::max()};
    coordinates::precision max_x{0};
    coordinates::precision max_y{0};

    for (auto const& e : station->elements_) {
      auto const& e_coords = layout[e->id()];
      min_x = std::min(min_x, e_coords.x_);
      min_y = std::min(min_y, e_coords.y_);
      max_x = std::max(max_x, e_coords.x_);
      max_y = std::max(max_y, e_coords.y_);
    }

    // adjust the coordinates to the upper left
    for (auto const& e : station->elements_) {
      layout[e->id()].x_ -= min_x;
      layout[e->id()].y_ -= min_y;
    }
  }
}

layout get_layout(
    std::vector<utls::loaded_file> const& rail_plan_files,
    soro::vector<station::ptr> const& stations,
    soro::vector<section> const& sections,
    soro::map<rail_plan_node_id, element_id> const& rp_id_to_element_id,
    std::size_t const element_count) {
  uLOG(info) << "[ Layout ] Determining layout from ISS.";

  layout layout(element_count);

  // gather all coordinates from the railplan files
  for (auto const& rail_plan_file : rail_plan_files) {
    pugi::xml_document d;
    auto success = d.load_buffer(
        reinterpret_cast<void const*>(rail_plan_file.contents_.data()),
        rail_plan_file.contents_.size());
    utl::verify(success, "Bad xml in get_layout: {}", success.description());

    for (auto const& rp_station : d.child(XML_ISS_DATA)
                                      .child(RAIL_PLAN_STATIONS)
                                      .children(RAIL_PLAN_STATION)) {
      for (auto const& rp_section :
           rp_station.child(RAIL_PLAN_SECTIONS).children(RAIL_PLAN_SECTION)) {

        for (auto const& rp_node :
             rp_section.child(RAIL_PLAN_NODE).children()) {

          if (utls::equal(rp_node.name(), "Bildpunkt")) {
            continue;
          }

          if (auto const& coord_child = rp_node.child(PICTURE_COORDINATES);
              static_cast<bool>(coord_child)) {
            auto const rp_id = std::stoull(rp_node.child_value(ID));
            layout[rp_id_to_element_id.at(rp_id)] =
                parse_coordinates(coord_child);
          }
        }
      }
    }
  }

  calculate_track_element_coords(layout, sections);
  adjust_coordinates(layout, stations);

  return layout;
}

}  // namespace soro::layout
