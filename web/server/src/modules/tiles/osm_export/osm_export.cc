#include "soro/server/modules/tiles/osm_export/osm_export.h"

#include <cstddef>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <future>
#include <map>
#include <thread>
#include <utility>
#include <vector>

#include "utl/logging.h"
#include "utl/overloaded.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/coordinates/gps.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/station/station.h"

#include "soro/server/modules/infrastructure/positions.h"

#include "soro/server/modules/tiles/osm_export/interpolation.h"
#include "soro/server/modules/tiles/osm_export/osm_tag_names.h"

using namespace utl;

using namespace soro::utls;
using namespace soro::infra;

namespace soro::server::osm_export {

/**
 * elem_ the element
 * station_ the station the element belongs to
 */
struct osm_element {
  element::ptr elem_;
  station::ptr station_;
};

struct osm_node {
  node::ptr node_;
  station::ptr station_;
};

struct osm_information {

  // all nodes in the xml
  // size_t id
  std::vector<std::pair<size_t, soro::variant<station::ptr, osm_element>>>
      nodes_;

  // the ways in the xml, between two elements in nodes_
  std::vector<std::pair<element::id, element::id>> ways_;

  // the interpolations
  std::vector<interpolation> interpolations_;

  // the connection between nodes
  // prevents duplicate ways
  std::map<element::id, std::vector<element::id>> element_to_way_;

  // key: id of border element
  // value: id of border element in other station and xml id of interpolation
  // nodes
  // can be used for signal station routes
  std::map<element::id, std::pair<element::id, std::vector<std::size_t>>>
      element_to_interpolation_nodes_;
  // ids of elements in station route, id of station_route_way
  std::vector<std::pair<std::vector<size_t>, size_t>> station_route_nodes_;
};

}  // namespace soro::server::osm_export

namespace soro::server::osm_export::detail {

void osm_add_key_value(auto const& key, auto const& value,
                       pugi::xml_node& node) {
  node.append_attribute(key).set_value(value);
}

void osm_add_tag(auto const& key, auto const& value, auto& parent_node) {
  auto tag_node = parent_node.append_child(tag_str);
  osm_add_key_value(k_str, key, tag_node);  // NOLINT
  osm_add_key_value(v_str, value, tag_node);
}

void osm_add_node_with_key_value(auto const& key, auto const& value,
                                 auto const& node_name, auto& parent_node) {
  auto node = parent_node.append_child(node_name);
  osm_add_key_value(key, value, node);
}

void osm_add_coordinates(gps const& gps, auto& node) {
  osm_add_key_value(lon_str, gps.lon_, node);
  osm_add_key_value(lat_str, gps.lat_, node);
}

void create_station_osm(
    pugi::xml_node& osm, station::ptr station, std::size_t const osm_id,
    soro::vector_map<station::id, gps> const& station_coords) {
  auto station_node = osm.append_child(node_str);
  station_node.append_attribute(id_str).set_value(osm_id);
  osm_add_coordinates(station_coords[station->id_], station_node);
  osm_add_tag(type_str, station_str, station_node);
  osm_add_tag(id_str, as_val(station->id_), station_node);
  osm_add_tag(name_str, station->ds100_.data(), station_node);
}

void create_element_osm(
    pugi::xml_node& osm, element::ptr const e,
    soro::vector_map<element::id, gps> const& element_coords) {

  auto element_node = osm.append_child(node_str);

  element_node.append_attribute(id_str).set_value(as_val(e->get_id()));

  osm_add_coordinates(element_coords[e->get_id()], element_node);

  osm_add_tag(type_str, element_str, element_node);
  osm_add_tag(subtype_str, e->get_type_str().c_str(), element_node);
  osm_add_tag(id_str, as_val(e->get_id()), element_node);

  if (e->is_track_element()) {
    osm_add_tag(direction_str,
                e->as<track_element>().rising() ? rising_str : falling_str,
                element_node);
  }
}

void create_way_osm(pugi::xml_node& osm_node, element::id const first,
                    element::id const second, size_t const id) {
  auto way = osm_node.append_child(way_str);
  way.append_attribute(id_str).set_value(id);
  osm_add_node_with_key_value(ref_str, as_val(first), nd_str, way);
  osm_add_node_with_key_value(ref_str, as_val(second), nd_str, way);
  osm_add_tag(railway_str, rail_str, way);
}

void create_ways(auto& osm_info, infrastructure const& infra,
                 element::ptr const e, positions const& positions) {
  auto const e_station = infra->element_to_station_.at(e->get_id());

  for (auto neigh : e->neighbours()) {
    if (neigh == nullptr) {
      continue;
    }

    if (osm_info.element_to_way_.contains(neigh->get_id())) {
      auto vec = osm_info.element_to_way_.at(neigh->get_id());
      if (std::find(vec.begin(), vec.end(), e->get_id()) != vec.end()) {
        continue;
      }
    }

    auto n_station = infra->element_to_station_.at(neigh->get_id());
    if (n_station != e_station) {
      osm_info.interpolations_.push_back(
          compute_interpolation(e, neigh, positions.elements_));
      osm_info.element_to_way_[neigh->get_id()].push_back(e->get_id());
      osm_info.element_to_way_[e->get_id()].push_back(neigh->get_id());
    } else {
      osm_info.ways_.emplace_back(std::pair(e->get_id(), neigh->get_id()));
      osm_info.element_to_way_[neigh->get_id()].push_back(e->get_id());
      osm_info.element_to_way_[e->get_id()].push_back(neigh->get_id());
    }
  }
}

pugi::xml_document export_osm_nodes(std::size_t const min,
                                    std::size_t const max,
                                    osm_information& osm_info,
                                    positions const& positions) {
  pugi::xml_document osm_node;

  for (std::size_t i = min; i < max; i++) {
    auto val = osm_info.nodes_.at(i);
    val.second.apply(utl::overloaded{
        [&](station::ptr const s) {
          create_station_osm(osm_node, s, val.first, positions.stations_);
        },
        [&](osm_element const& e) {
          create_element_osm(osm_node, e.elem_, positions.elements_);
        }});
  }

  return osm_node;
}

}  // namespace soro::server::osm_export::detail

namespace soro::server::osm_export {

void append_fragment(pugi::xml_node target,
                     pugi::xml_document const& cached_fragment) {
  for (pugi::xml_node child = cached_fragment.first_child(); child != nullptr;
       child = child.next_sibling()) {
    target.append_copy(child);
  }
}

pugi::xml_document export_to_osm(infrastructure const& infra,
                                 positions const& positions) {
  utl::scoped_timer const timer("exporting osm");

  pugi::xml_document document;
  pugi::xml_node osm_node = document.append_child("osm");
  auto version = osm_node.append_attribute("version");
  version.set_value("0.6");

  osm_information osm_info;

  uLOG(info) << "[ OSM Export ] Parsing stations.";
  auto const station_id_offset = infra->graph_.elements_.size();
  for (auto const& station : infra->stations_) {
    auto const station_id = station->id_ + station_id_offset;
    osm_info.nodes_.emplace_back(std::pair(as_val(station_id), station));
    for (auto const& elem : station->elements_) {
      osm_element const osm_elem = {elem, station};
      osm_info.nodes_.emplace_back(std::pair(as_val(elem->get_id()), osm_elem));
      detail::create_ways(osm_info, infra, elem, positions);
    }
  }

  uLOG(info) << "[ OSM Export ] Sorting stations.";
  std::sort(osm_info.nodes_.begin(), osm_info.nodes_.end(),
            [](auto& left, auto& right) { return left.first < right.first; });

  uLOG(info) << "[ OSM Export ] Stations and elements to OSM.";
  auto node_length = osm_info.nodes_.size();
  std::size_t const thread_count = std::thread::hardware_concurrency();
  std::size_t const unit = (node_length / thread_count) + 1;
  std::vector<std::future<pugi::xml_document>> docs;

  for (auto i = 0UL; i < thread_count; i++) {
    auto min = unit * i;
    auto max = unit * (i + 1);
    if (max > node_length) {
      max = node_length;
    }
    docs.push_back(std::async(&detail::export_osm_nodes, min, max,
                              std::ref(osm_info), positions));
  }
  for (auto& e : docs) {
    append_fragment(osm_node, e.get());
  }

  auto id = osm_info.nodes_.back().first + 1;

  uLOG(info) << "[ OSM Export ] Ways to OSM.";
  for (auto way : osm_info.ways_) {
    detail::create_way_osm(osm_node, way.first, way.second, id++);
  }

  return document;
}

void write_osm_to_file(pugi::xml_document const& osm_xml,
                       std::filesystem::path const& out) {
  utl::scoped_timer const timer("writing osm file");

  osm_xml.save_file(out.c_str());

  uLOG(info) << "exported file successfully to " << out;
}

void export_and_write(infrastructure const& infra, positions const& positions,
                      std::filesystem::path const& out) {
  write_osm_to_file(export_to_osm(infra, positions), out);
}

}  // namespace soro::server::osm_export
