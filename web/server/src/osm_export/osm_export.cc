#include "soro/server/osm_export/osm_export.h"

#include <future>

#include "utl/logging.h"
#include "utl/overloaded.h"
#include "utl/progress_tracker.h"

#include "soro/server/osm_export/interpolation.h"
#include "soro/server/osm_export/osm_tag_names.h"

using namespace utl;

using namespace soro::utls;
using namespace soro::infra;

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

void create_station_osm(pugi::xml_node& osm, station::ptr station,
                        std::size_t const osm_id,
                        soro::vector<gps> const& station_coords) {
  auto station_node = osm.append_child(node_str);
  station_node.append_attribute(id_str).set_value(osm_id);
  osm_add_coordinates(station_coords[station->id_], station_node);
  osm_add_tag(type_str, station_str, station_node);
  osm_add_tag(id_str, station->id_, station_node);
  osm_add_tag(name_str, station->ds100_.data(), station_node);
}

void create_element_osm(pugi::xml_node& osm, element_ptr e,
                        soro::vector<gps> const& element_coords) {
  auto element_node = osm.append_child(node_str);
  element_node.append_attribute(id_str).set_value(e->id());
  osm_add_coordinates(element_coords[e->id()], element_node);
  osm_add_tag(type_str, element_str, element_node);
  osm_add_tag(subtype_str, e->get_type_str().c_str(), element_node);
  osm_add_tag(id_str, e->id(), element_node);
  osm_add_tag(direction_str, e->rising() ? rising_str : falling_str,
              element_node);
}

void create_way_osm(pugi::xml_node& osm_node, element_id const first,
                    element_id const second, size_t const id) {
  auto way = osm_node.append_child(way_str);
  way.append_attribute(id_str).set_value(id);
  osm_add_node_with_key_value(ref_str, first, nd_str, way);
  osm_add_node_with_key_value(ref_str, second, nd_str, way);
  osm_add_tag(railway_str, rail_str, way);
}

std::size_t create_interpolation_osm(auto osm, auto const& interpolation,
                                     auto id, auto& osm_info) {
  std::vector<size_t> ids;
  osm_info.ways_.emplace_back(std::pair(interpolation.first_elem_, id));
  for (auto i = 0UL; i < interpolation.points_.size(); i++) {
    if (i < interpolation.points_.size() - 1) {
      osm_info.ways_.emplace_back(std::pair(id + i, id + i + 1));
    }
    ids.push_back(id + i);
    auto auxiliary_node = osm.append_child("node");
    auxiliary_node.append_attribute("id").set_value(id + i);
    auto auxiliary_coords = interpolation.points_.at(i);
    osm_add_coordinates(auxiliary_coords, auxiliary_node);
    osm_add_tag(type_str, interpolation_str, auxiliary_node);
  }
  osm_info.ways_.emplace_back(std::pair(id + interpolation.points_.size() - 1,
                                        interpolation.second_elem_));
  osm_info.element_to_interpolation_nodes_[interpolation.first_elem_] =
      std::make_pair(interpolation.second_elem_, ids);
  return id + interpolation.points_.size();
}

void create_ways(auto& osm_info, infrastructure_t const& iss, element_ptr e) {
  auto const e_station = iss.element_to_station_.at(e->id());

  for (auto neigh : e->neighbours()) {
    if (neigh == nullptr) {
      continue;
    }

    if (osm_info.element_to_way_.contains(neigh->id())) {
      auto vec = osm_info.element_to_way_.at(neigh->id());
      if (std::find(vec.begin(), vec.end(), e->id()) != vec.end()) {
        continue;
      }
    }

    auto n_station = iss.element_to_station_.at(neigh->id());
    if (n_station != e_station) {
      osm_info.interpolations_.push_back(
          compute_interpolation(e, neigh, iss.element_positions_));
      osm_info.element_to_way_[neigh->id()].push_back(e->id());
      osm_info.element_to_way_[e->id()].push_back(neigh->id());
    } else {
      osm_info.ways_.emplace_back(std::pair(e->id(), neigh->id()));
      osm_info.element_to_way_[neigh->id()].push_back(e->id());
      osm_info.element_to_way_[e->id()].push_back(neigh->id());
    }
  }
}

pugi::xml_document export_osm_nodes(std::size_t const min,
                                    std::size_t const max,
                                    infrastructure_t const& iss,
                                    osm_information& osm_info) {
  pugi::xml_document osm_node;
  for (std::size_t i = min; i < max; i++) {
    auto val = osm_info.nodes_.at(i);
    val.second.apply(utl::overloaded{
        [&](station::ptr s) {
          create_station_osm(osm_node, s, val.first, iss.station_positions_);
        },
        [&](osm_element const& e) {
          create_element_osm(osm_node, e.elem_, iss.element_positions_);
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

pugi::xml_document export_to_osm(soro::infra::infrastructure_t const& iss) {
  uLOG(info) << "[ OSM Export ] Starting OSM export.";
  pugi::xml_document document;
  pugi::xml_node osm_node = document.append_child("osm");
  auto version = osm_node.append_attribute("version");
  version.set_value("0.6");

  osm_information osm_info;

  uLOG(info) << "[ OSM Export ] Parsing stations.";
  auto const station_id_offset = iss.graph_.elements_.size();
  for (auto station : iss.stations_) {
    size_t const station_id = station->id_ + station_id_offset;
    osm_info.nodes_.emplace_back(std::pair(station_id, station));
    for (auto elem : station->elements_) {
      osm_element const osm_elem = {elem, station};
      osm_info.nodes_.emplace_back(std::pair(elem->id(), osm_elem));
      detail::create_ways(osm_info, iss, elem);
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
                              std::cref(iss), std::ref(osm_info)));
  }
  for (auto& e : docs) {
    append_fragment(osm_node, e.get());
  }

  auto id = osm_info.nodes_.back().first + 1;

  uLOG(info) << "[ OSM Export ] Interpolations to OSM.";
  for (const auto& interpolation : osm_info.interpolations_) {
    id =
        detail::create_interpolation_osm(osm_node, interpolation, id, osm_info);
  }

  uLOG(info) << "[ OSM Export ] Ways to OSM.";
  for (auto way : osm_info.ways_) {
    detail::create_way_osm(osm_node, way.first, way.second, id++);
  }

  return document;
}

void write_osm_to_file(pugi::xml_document const& osm_xml, fs::path const& out) {
  osm_xml.save_file(out.c_str());
  uLOG(info) << "[ OSM Export ] Exported file successfully to " << out << ".";
}

void export_and_write(infrastructure_t const& iss, fs::path const& out) {
  write_osm_to_file(export_to_osm(iss), out);
}

}  // namespace soro::server::osm_export
