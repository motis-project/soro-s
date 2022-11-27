#include "soro/infrastructure/parsers/iss/parse_iss.h"

#include <cstdint>
#include <string>
#include <utility>

#include "pugixml.hpp"

#include "utl/enumerate.h"
#include "utl/erase_if.h"
#include "utl/logging.h"
#include "utl/pipes.h"
#include "utl/timer.h"
#include "utl/verify.h"

#include "soro/utls/execute_if.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/graph/graph_creation.h"
#include "soro/infrastructure/interlocking/get_interlocking_subsystem.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/layout/layout.h"
#include "soro/infrastructure/layout/layout_to_gps.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/parse_helpers.h"
#include "soro/infrastructure/parsers/iss/parse_station_route.h"
#include "soro/infrastructure/parsers/iss/parse_track_element.h"
#include "soro/infrastructure/parsers/parse_station_coords.h"
#include "soro/infrastructure/path/length.h"
#include "soro/infrastructure/regulatory_data.h"

#include "soro/rolling_stock/parse_train_series.h"

namespace soro::infra {

using namespace pugi;
using namespace utl;
using namespace soro::si;
using namespace soro::utls;

length get_section_length(xml_node const& section_start,
                          xml_node const& section_end) {
  auto const start_km = parse_kilometrage(section_start);
  auto const end_km = parse_kilometrage(section_end);

  return abs(start_km - end_km);
}

border parse_border(xml_node const& xml_rp_border, element* border_element,
                    line_id const line, bool const is_start) {
  border b;

  b.neighbour_name_ = xml_rp_border.child_value(PARTNER_STATION);
  b.track_sign_ = std::stoi(xml_rp_border.child_value(TRACK_SIGN));
  b.line_ = line;
  b.element_ = border_element;
  b.low_border_ = is_start;

  return b;
}

section::id parse_section_into_network(xml_node const& xml_rp_section,
                                       station& station, graph& network,
                                       construction_materials& mats) {
  line_id line =
      static_cast<uint32_t>(std::stoul(xml_rp_section.child_value(LINE)));
  auto const& section_start =
      xml_rp_section.child(RAIL_PLAN_NODE).children().begin();
  auto const& section_end =
      std::prev(xml_rp_section.child(RAIL_PLAN_NODE).children().end());

  auto const section_id = create_section(network);
  auto& sec = network.sections_[section_id];
  sec.length_ = get_section_length(*section_start, *section_end);
  sec.line_id_ = line;

  auto get_main_rp_node_id = [](auto const& node) -> rail_plan_node_id {
    switch (str_hash(node.name())) {
      case str_hash(BUMPER):
      case str_hash(BORDER):
      case str_hash(TRACK_END):
      case str_hash(KM_JUMP_START):
      case str_hash(SWITCH_START):
      case str_hash(CROSS_START_LEFT):
      case str_hash(CROSS_SWITCH_START_LEFT):
      case str_hash(LINE_SWITCH_ZERO): {
        return parse_rp_node_id(node);
      }

      case str_hash(KM_JUMP_END):
      case str_hash(LINE_SWITCH_ONE): {
        return std::stoull(node.child(PARTNER_NODE).attribute(ID).value());
      }

      case str_hash(SWITCH_STEM):
      case str_hash(SWITCH_BRANCH_LEFT):
      case str_hash(SWITCH_BRANCH_RIGHT): {
        return std::stoull(node.child(PARTNER).attribute(ID).value());
      }

      case str_hash(CROSS_END_LEFT):
      case str_hash(CROSS_SWITCH_END_LEFT): {
        return std::stoull(node.child(NEIGHBOUR_AHEAD).attribute(ID).value());
      }

      case str_hash(CROSS_START_RIGHT):
      case str_hash(CROSS_END_RIGHT):
      case str_hash(CROSS_SWITCH_START_RIGHT):
      case str_hash(CROSS_SWITCH_END_RIGHT): {
        return std::stoull(node.child(MAIN_NODE_START).attribute(ID).value());
      }

      default: {
        throw utl::fail(
            "Could not get main rail plan node for node with name: {}",
            node.name());
      }
    }
  };

  auto emplace_into_network = [&](auto const& node, bool const is_start) {
    auto const rp_id = parse_rp_node_id(node);
    auto const main_rp_id = get_main_rp_node_id(node);

    auto element = get_or_create_element(
        network, station, mats, get_type(node.name()), main_rp_id, false);

    if (rp_id == main_rp_id) {
      element->e_.apply([&is_start](auto&& e) { e.rising_ = !is_start; });
    } else {
      mats.rp_id_to_element_id_[rp_id] = element->id();
    }

    if (utls::equal(node.name(), LINE_SWITCH_ONE)) {
      element->template as<simple_element>().end_rising_ = !is_start;
    }

    if (utls::equal(node.name(), SWITCH_STEM)) {
      element->template as<simple_switch>().stem_rising_ = !is_start;
    }

    if (utls::equal(node.name(), SWITCH_BRANCH_RIGHT) ||
        utls::equal(node.name(), SWITCH_BRANCH_LEFT)) {
      element->template as<simple_switch>().branch_rising_ = !is_start;
    }

    if (utls::equal(node.name(), CROSS_END_LEFT) ||
        utls::equal(node.name(), CROSS_SWITCH_END_LEFT)) {
      element->template as<cross>().end_left_rising_ = !is_start;
    }

    if (utls::equal(node.name(), CROSS_START_RIGHT) ||
        utls::equal(node.name(), CROSS_SWITCH_START_RIGHT)) {
      element->template as<cross>().start_right_rising_ = !is_start;
    }

    if (utls::equal(node.name(), CROSS_END_RIGHT) ||
        utls::equal(node.name(), CROSS_SWITCH_END_RIGHT)) {
      element->template as<cross>().end_right_rising_ = !is_start;
    }

    if (utls::equal(node.name(), CROSS_SWITCH_START_LEFT)) {
      element->template as<cross>().start_left_end_right_arc_ =
          node.child(NEIGHBOUR_BRANCH);
    }

    if (utls::equal(node.name(), CROSS_SWITCH_END_LEFT)) {
      element->template as<cross>().start_right_end_left_arc_ =
          node.child(NEIGHBOUR_BRANCH);
    }

    if (utls::equal(node.name(), BORDER)) {
      station.borders_.push_back(parse_border(node, element, line, is_start));
    }

    return element;
  };

  auto start_element = emplace_into_network(*section_start, true);
  auto end_element = emplace_into_network(*section_end, false);

  set_km_point_and_line(*start_element, section_start->name(),
                        parse_kilometrage(*section_start), line);
  set_km_point_and_line(*end_element, section_end->name(),
                        parse_kilometrage(*section_end), line);

  sec.elements_.emplace_back(start_element);
  network.element_id_to_section_ids_[start_element->id()].push_back(section_id);

  auto prev_rising_element = start_element;
  auto prev_falling_element = start_element;

  auto prev_rising_rp_node = *section_start;
  auto prev_falling_rp_node = *section_start;

  for (auto const& node : xml_rp_section.child(RAIL_PLAN_NODE).children()) {
    auto const type = get_type(node.name());

    if (is_directed_track_element(type)) {
      auto const rising = has_rising_name(node);

      auto& prev_element = rising ? prev_rising_element : prev_falling_element;
      auto& prev_node = rising ? prev_rising_rp_node : prev_falling_rp_node;

      auto const track_element =
          parse_track_element(node, type, rising, line, network, station, mats);

      set_neighbour(*prev_element, prev_node.name(), track_element, rising);
      set_neighbour(*track_element, node.name(), prev_element, !rising);

      sec.elements_.emplace_back(track_element);
      network.element_id_to_section_ids_[track_element->id()].push_back(
          section_id);

      prev_element = track_element;
      prev_node = node;
    } else if (is_undirected_track_element(type)) {
      auto const track_element =
          parse_track_element(node, type, true, line, network, station, mats);

      set_neighbour(*prev_rising_element, prev_rising_rp_node.name(),
                    track_element, true);
      set_neighbour(*prev_falling_element, prev_falling_rp_node.name(),
                    track_element, false);

      set_neighbour(*track_element, node.name(), prev_rising_element, true);
      set_neighbour(*track_element, node.name(), prev_falling_element, false);

      sec.elements_.emplace_back(track_element);
      network.element_id_to_section_ids_[track_element->id()].push_back(
          section_id);

      prev_rising_element = track_element;
      prev_falling_element = track_element;

      prev_rising_rp_node = node;
      prev_falling_rp_node = node;
    }
  }

  set_neighbour(*prev_rising_element, prev_rising_rp_node.name(), end_element,
                true);
  set_neighbour(*prev_falling_element, prev_falling_rp_node.name(), end_element,
                false);
  set_neighbour(*end_element, section_end->name(), prev_rising_element, true);
  set_neighbour(*end_element, section_end->name(), prev_falling_element, false);

  sec.elements_.emplace_back(end_element);
  network.element_id_to_section_ids_[end_element->id()].push_back(section_id);

  return section_id;
}

std::set<rail_plan_node_id> get_omitted_relevant_rp_nodes(
    intermediate_station_route const& isr1) {
  type_set const INTERLOCKING_DISCERNING_TYPES = {type::MAIN_SIGNAL};

  std::set<rail_plan_node_id> omitted_nodes;

  for (auto const& omitted : isr1.omitted_rp_nodes_) {
    if (INTERLOCKING_DISCERNING_TYPES.contains(omitted.type_)) {
      omitted_nodes.insert(omitted.rp_node_id_);
    }
  }

  return omitted_nodes;
}

bool same_path(intermediate_station_route const& isr1,
               intermediate_station_route const& isr2) {
  auto const same_start = isr1.start_ == isr2.start_;
  auto const same_end = isr1.end_ == isr2.end_;
  auto const same_course = isr1.course_ == isr2.course_;

  auto const same_omitted = get_omitted_relevant_rp_nodes(isr1) ==
                            get_omitted_relevant_rp_nodes(isr2);

  return same_start && same_end && same_course && same_omitted;
}

struct deduplicated_paths {
  soro::vector<station_route::path::ptr> paths_;
  soro::vector<soro::unique_ptr<station_route::path>> path_store_;
  soro::vector<station_route::path::ptr> station_route_id_to_path_id_;
};

deduplicated_paths get_station_route_paths(infrastructure_t const& infra,
                                           construction_materials const& mats) {
  utl::scoped_timer timer("Deduplicating Paths");

  auto const get_path = [&](intermediate_station_route const& sr,
                            node::ptr next_node, auto const last_node_id) {
    auto course_idx = 0UL;
    soro::vector<node::ptr> nodes;
    while (next_node != nullptr && next_node->id_ != last_node_id) {
      nodes.push_back(next_node);

      auto const switch_element = next_node->element_->is_switch();

      if (next_node->branch_node_ == nullptr ||
          sr.course_[course_idx] == course_decision::STEM) {
        next_node = next_node->next_node_;
      } else {
        next_node = next_node->branch_node_;
      }

      course_idx += switch_element ? 1 : 0;
    }

    utl::verify(next_node != nullptr,
                "Could not find path for station route {}", sr.name_);

    nodes.push_back(next_node);
    return nodes;
  };

  auto const get_node = [](element::ptr e, bool const start) {
    if (e->is(type::BORDER)) {
      auto const& border = e->as<simple_element>();
      auto ret = start ? (e->rising() ? border.bot() : border.top())
                       : (e->rising() ? border.top() : border.bot());
      return ret;
    } else if (e->is_end_element()) {
      auto const& end = e->as<end_element>();
      auto ret = start ? (end.rising_ ? end.bot() : end.top())
                       : (end.rising_ ? end.top() : end.bot());
      return ret;
    } else if (e->is_track_element()) {
      return e->as<track_element>().get_node();
    }

    throw utl::fail(
        "Station route does not start with border, end or track element");
  };

  auto const get_main_signals = [&](intermediate_station_route const& isr,
                                    std::vector<node::ptr> const& nodes) {
    std::set<node::ptr> omitted_main_signals;

    for (auto const& omitted : isr.omitted_rp_nodes_) {
      if (omitted.type_ == type::MAIN_SIGNAL) {
        auto const element_id =
            mats.rp_id_to_element_id_.at(omitted.rp_node_id_);
        omitted_main_signals.insert(
            infra.graph_.elements_[element_id]->as<track_element>().get_node());
      }
    }

    soro::vector<node::idx> main_signals;
    for (auto idx = 0UL; idx < nodes.size(); ++idx) {
      auto const node = nodes[idx];
      if (node->is(type::MAIN_SIGNAL) && !omitted_main_signals.contains(node)) {
        main_signals.emplace_back(static_cast<node::idx>(idx));
      }
    }

    return main_signals;
  };

  auto const& graph = infra.graph_;

  deduplicated_paths result;
  result.station_route_id_to_path_id_.resize(
      mats.intermediate_station_routes_.size());

  // this is just a helper data structure to help finding identical paths
  // it maps the following: element::id -> [{sr_id, path::ptr}]
  std::vector<std::vector<std::pair<std::size_t, station_route::path::ptr>>>
      start_elements_to_isr_paths(graph.elements_.size());

  for (auto const& i_sr : mats.intermediate_station_routes_) {
    auto start = graph.elements_[mats.rp_id_to_element_id_.at(i_sr.start_)];
    auto end = graph.elements_[mats.rp_id_to_element_id_.at(i_sr.end_)];

    // check if there already exists an identical path
    auto path_it = utls::find_if(
        start_elements_to_isr_paths[start->id()], [&](auto&& pair) {
          return same_path(mats.intermediate_station_routes_[pair.first], i_sr);
        });

    if (path_it != std::end(start_elements_to_isr_paths[start->id()])) {
      // we found an identical path, use it
      result.station_route_id_to_path_id_[i_sr.id_] = path_it->second;
      continue;
    }

    // create new path
    auto nodes =
        get_path(i_sr, get_node(start, true), get_node(end, false)->id_);
    auto main_signals = get_main_signals(i_sr, nodes);

    result.path_store_.emplace_back();
    result.path_store_.back() = soro::make_unique<station_route::path>(
        station_route::path{.start_ = start,
                            .end_ = end,
                            .course_ = i_sr.course_,
                            .nodes_ = std::move(nodes),
                            .main_signals_ = std::move(main_signals)});
    auto const path_ptr = result.path_store_.back().get();
    result.paths_.emplace_back(path_ptr);

    result.station_route_id_to_path_id_[i_sr.id_] = path_ptr;
    start_elements_to_isr_paths[start->id()].emplace_back(i_sr.id_, path_ptr);
  }

  return result;
}

void calculate_station_routes(infrastructure_t& infra,
                              construction_materials const& mats) {
  utl::scoped_timer const calc_timer("Calculating Station Routes");

  auto deduplicated_paths = get_station_route_paths(infra, mats);
  infra.station_route_path_store_ = std::move(deduplicated_paths.path_store_);
  infra.station_route_paths_ = std::move(deduplicated_paths.paths_);

  auto const& graph = infra.graph_;

  size_t through_routes = 0;
  size_t in_routes = 0;
  size_t out_routes = 0;

  infra.station_route_store_.reserve(mats.intermediate_station_routes_.size());
  for (auto const& i_sr : mats.intermediate_station_routes_) {
    infra.station_route_store_.emplace_back();
    sassert(infra.station_route_store_.size() == i_sr.id_ + 1,
            "Did not allocate enough space for the signal station route");

    infra.station_route_store_[i_sr.id_] = soro::make_unique<station_route>();
    auto sr = infra.station_route_store_[i_sr.id_].get();
    infra.station_routes_.emplace_back(sr);

    sr->id_ = static_cast<station_route::id>(i_sr.id_);
    sr->path_ = deduplicated_paths.station_route_id_to_path_id_[i_sr.id_];
    sr->name_ = i_sr.name_;
    sr->station_ = i_sr.station_;
    sr->attributes_ = i_sr.attributes_;

    auto& station = infra.station_store_[i_sr.station_->id_];

    utls::sassert(
        station->station_routes_.find(i_sr.name_) ==
            std::end(station->station_routes_),
        "There is already a station route with the name {} in station {}",
        i_sr.name_, station->ds100_);

    station->station_routes_[i_sr.name_] = sr;

    if (sr->path_->start_->is_track_element()) {
      ++out_routes;
    } else if (sr->path_->end_->is_track_element()) {
      ++in_routes;
    } else {
      ++through_routes;
    }

    if (auto next = sr->path_->nodes_.back()->next_node_; next != nullptr) {
      auto const next_station =
          infra.element_to_station_.at(next->element_->id());
      sr->to_station_ = next_station != sr->station_ ? next_station : nullptr;
    }

    if (auto inc = sr->path_->nodes_.front()->reverse_edges_; !inc.empty()) {
      auto const prev_station =
          infra.element_to_station_.at(inc.front()->element_->id());
      sr->from_station_ = prev_station != sr->station_ ? prev_station : nullptr;
    }

    auto const get_halt_node = [&](auto&& rp_id) -> node::ptr {
      if (rp_id == INVALID_RP_NODE_ID) {
        return nullptr;
      }

      auto const e = graph.elements_[mats.rp_id_to_element_id_.at(rp_id)];
      return e->template as<track_element>().get_node();
    };

    auto const passenger_node = get_halt_node(i_sr.rp_passenger_halt_);
    auto const freight_node = get_halt_node(i_sr.rp_freight_halt_);

    std::set<node::ptr> omitted_nodes_set;
    for (auto const& omitted_rp_node : i_sr.omitted_rp_nodes_) {
      auto const omit_element = graph.elements_[mats.rp_id_to_element_id_.at(
          omitted_rp_node.rp_node_id_)];

      utl::verify(omit_element->is_track_element(),
                  "Cannot omit a non track element!");

      omitted_nodes_set.insert(omit_element->as<track_element>().get_node());
    }

    for (node::idx idx = 0; idx < sr->size(); ++idx) {
      auto const& node = sr->nodes(idx);

      if (!node->element_->is_track_element()) {
        continue;
      }

      if (node == passenger_node) {
        sr->passenger_halt_ = idx;
      }

      if (node == freight_node) {
        sr->freight_halt_ = idx;
      }

      if (omitted_nodes_set.contains(node)) {
        sr->omitted_nodes_.push_back(idx);
      }
    }

    sr->extra_speed_limits_ = i_sr.extra_speed_limits_;
    for (auto& spl : sr->extra_speed_limits_) {
      auto const it = utls::find_if(sr->nodes(), [&spl](auto&& n) {
        return n->element_->id() == spl.element_->id();
      });

      utls::sassert(it != std::end(sr->nodes()),
                    "Could not find node for the extra speed limit!");

      spl.node_ = *it;
    }

    utls::sort(sr->extra_speed_limits_,
               [&sr](auto const& spl1, auto const& spl2) {
                 auto n1 = utls::find_if(sr->nodes(), [&spl1](auto const& n) {
                   return n->element_->id() == spl1.element_->id();
                 });

                 auto n2 = utls::find_if(sr->nodes(), [&spl2](auto const& n) {
                   return n->element_->id() == spl2.element_->id();
                 });

                 return std::distance(std::cbegin(sr->nodes()), n1) <
                        std::distance(std::cbegin(sr->nodes()), n2);
               });

    sr->length_ = get_path_length_from_elements(sr->nodes());

    utls::sassert(
        sr->extra_speed_limits_.size() == i_sr.extra_speed_limits_.size(),
        "Did not account for every extra speed limit.");
    utls::sassert(sr->omitted_nodes_.size() == i_sr.omitted_rp_nodes_.size(),
                  "Did not account for every omitted node.");
  }

  // TODO(julian) refactor this into a separate function
  // fill element_to_station_routes_ map in every station
  for (auto& station : infra.station_store_) {
    for (auto const& [name, station_route] : station->station_routes_) {
      auto const first = station_route->nodes().front()->element_;
      auto it = station->element_to_routes_.find(first->id());
      if (it != std::end(station->element_to_routes_)) {
        it->second.push_back(station_route);
      } else {
        station->element_to_routes_[first->id()] = {station_route};
      }
    }
  }

  uLOG(info) << "Parsed " << infra.station_routes_.size() << " station routes";
  uLOG(info) << "Used " << infra.station_route_paths_.size() << " paths";
  uLOG(info) << through_routes << " through routes.";
  uLOG(info) << in_routes << " in routes.";
  uLOG(info) << out_routes << " out routes.";
}

soro::unique_ptr<station> parse_iss_station(xml_node const& rp_station,
                                            infrastructure_t& iss,
                                            construction_materials& mats,
                                            station::id const id) {
  auto station = soro::make_unique<struct station>();
  station->ds100_ = soro::string(rp_station.child_value(STATION));
  station->id_ = id;

  for (auto const& section :
       rp_station.child(RAIL_PLAN_SECTIONS).children(RAIL_PLAN_SECTION)) {
    auto const section_id =
        parse_section_into_network(section, *station, iss.graph_, mats);
    station->sections_.emplace_back(section_id);
  }

  for (auto const& xml_sr :
       rp_station.child(STATION_ROUTES).children(STATION_ROUTE)) {
    auto const sr_id = mats.intermediate_station_routes_.size();

    auto const publish_only = xml_sr.child(PUBLISH_ONLY);
    if (static_cast<bool>(publish_only)) {
      continue;
    }

    mats.intermediate_station_routes_.push_back(
        parse_station_route(sr_id, xml_sr, station.get(), iss.graph_, mats));
  }

  return station;
}

void complete_borders(infrastructure_t& iss) {
  sassert(!iss.station_store_.empty(), "Requires station already constructed");
  sassert(!iss.ds100_to_station_.empty(), "Requires DS100 to station mapping");

  size_t erased_borders = 0;

  for (auto& station : iss.station_store_) {
    for (auto& border : station->borders_) {
      auto it = iss.ds100_to_station_.find(border.neighbour_name_);
      if (it != std::end(iss.ds100_to_station_)) {
        border.neighbour_ = it->second;
      }

      border.station_ = station.get();
    }

    // remove a border if the bordering station is not in the
    // base_infrastructure dataset
    erased_borders += station->borders_.size();
    utl::erase_if(station->borders_,
                  [](auto&& border) { return border.neighbour_ == nullptr; });
    erased_borders -= station->borders_.size();
  }

  uLOG(info) << "Erased " << erased_borders << " borders.";

  for (auto& from : iss.station_store_) {
    for (auto& from_border : from->borders_) {
      for (auto const& to_border : from_border.neighbour_->borders_) {
        if (to_border.neighbour_ != from.get() ||
            from_border.track_sign_ != to_border.track_sign_ ||
            from_border.line_ != to_border.line_) {
          continue;
        }

        from_border.neighbour_element_ = to_border.element_;

#if defined(SERIALIZE)
        auto& non_const_border =
            static_cast<non_const_element_ptr>(from_border.element_)
                ->as<simple_element>();
#else
        auto& non_const_border =
            const_cast<non_const_element_ptr>(from_border.element_)  // NOLINT
                ->as<simple_element>();

#endif
        connect_border(non_const_border, from_border.low_border_,
                       to_border.element_);
      }
    }
  }

  // insert empty sections between two borders of neighbouring stations,
  // to make the section model consistent
  soro::set<border::id_tuple> finished_borders;

  for (auto const& station : iss.stations_) {
    for (auto const& border : station->borders_) {
      auto const id_tuple = border.get_id_tuple();

      if (finished_borders.find(id_tuple) != std::end(finished_borders)) {
        // border pair is already finished
        continue;
      }

      auto const new_sec_id = create_section(iss.graph_);
      auto& new_sec = iss.graph_.sections_[new_sec_id];

      new_sec.line_id_ = border.line_;

      if (border.low_border_) {
        new_sec.elements_ = {border.neighbour_element_, border.element_};
      } else {
        new_sec.elements_ = {border.element_, border.neighbour_element_};
      }

      iss.graph_.element_id_to_section_ids_[border.element_->id()].push_back(
          new_sec_id);
      iss.graph_.element_id_to_section_ids_[border.neighbour_element_->id()]
          .push_back(new_sec_id);

      finished_borders.insert(id_tuple);
    }
  }
}

auto get_element_to_station_map(infrastructure_t const& iss) {
  soro::map<element_id, station::ptr> element_to_station;

  for (auto const& station : iss.stations_) {
    for (auto const& element : station->elements_) {
      element_to_station[element->id()] = station;
    }
  }

  return element_to_station;
}

soro::map<soro::string, station::ptr> get_ds100_to_station(
    soro::vector<station::ptr> const& stations) {
  return utl::all(stations) | utl::transform([](auto&& s_ptr) {
           return soro::pair<soro::string, station::ptr>{s_ptr->ds100_, s_ptr};
         }) |
         utl::emplace<soro::map<string, station::ptr>>();
}

void log_stats(infrastructure_t const& iss) {
  uLOG(info) << "Parsed ISS with " << iss.stations_.size() << " stations.";
  uLOG(info) << "Parsed ISS with " << iss.graph_.elements_.size()
             << " elements and " << iss.graph_.nodes_.size() << " nodes.";

  std::size_t total_speed_limits = 0;
  std::size_t spl_with_length = 0;
  std::size_t spl_with_signal_poa = 0;
  std::size_t spl_with_here_poa = 0;

  for (auto const& data : iss.graph_.element_data_) {
    execute_if<speed_limit>(data, [&](auto&& spl) {
      ++total_speed_limits;
      spl_with_length += valid(spl.length_) ? 1 : 0;
      spl_with_signal_poa += spl.poa_ == speed_limit::poa::LAST_SIGNAL ? 1 : 0;
      spl_with_here_poa += spl.poa_ == speed_limit::poa::HERE ? 1 : 0;
    });
  }

  uLOG(info) << "Number of speed limits: " << total_speed_limits;
  uLOG(info) << "Speed limits with length: " << spl_with_length;
  uLOG(info) << "Speed limits with 'last signal' point of action: "
             << spl_with_signal_poa;
  uLOG(info) << "Speed limits with 'here' point of action: "
             << spl_with_here_poa;

  std::size_t total_section_elements = 0;
  std::size_t total_main_signals = 0;
  std::size_t total_simple_switches = 0;
  std::size_t total_crosses = 0;
  std::size_t total_cross_switches = 0;
  std::size_t switchables = 0;
  for (auto const& element : iss.graph_.elements_) {
    if (element->is(type::MAIN_SIGNAL)) {
      ++total_main_signals;
    }

    if (element->is_section_element()) {
      ++total_section_elements;
    }

    if (element->is_switch()) {
      ++switchables;
    }

    if (element->is(type::CROSS) && !element->is_cross_switch()) {
      ++total_crosses;
    }

    if (element->is_cross_switch()) {
      ++total_cross_switches;
    }

    if (element->is(type::SIMPLE_SWITCH)) {
      ++total_simple_switches;
    }
  }

  uLOG(info) << "Got " << total_main_signals << " main signals.";
  uLOG(info) << "Got " << total_section_elements << " section elements.";
  uLOG(info) << "Got " << total_simple_switches << " simple switches.";
  uLOG(info) << "Got " << total_crosses << " crosses.";
  uLOG(info) << "Got " << total_cross_switches << " cross switches.";
  uLOG(info) << "Got " << switchables << " total switchables.";
}

default_values parse_default_values(xml_node const& default_values_xml) {
  default_values dv;

  dv.line_class_ = default_values_xml.child_value(LINE_CLASS);
  dv.route_form_time_ = si::from_s(parse_fp<si::precision, replace_comma::ON>(
      default_values_xml.child_value(ROUTE_FORM_TIME)));
  dv.brake_path_length_ = si::from_m(parse_fp<si::precision, replace_comma::ON>(
      default_values_xml.child_value(BRAKE_PATH_LENGTH)));
  dv.stationary_speed_limit_ = si::from_km_h(parse_fp<si::precision>(
      default_values_xml.child_value(STATIONARY_SPEED_LIMIT)));

  return dv;
}

std::pair<default_values, rs::rolling_stock> parse_core_data(
    std::vector<utls::loaded_file> const& core_files) {
  utl::scoped_timer const core_data_timer("Parsing Core Data");

  default_values dv;
  rs::rolling_stock rs;

  for (auto const& core_file : core_files) {
    uLOG(utl::info) << "Parsing core data file " << core_file.path_;

    xml_document d;
    auto success =
        d.load_buffer(reinterpret_cast<void const*>(core_file.contents_.data()),
                      core_file.contents_.size());
    utl::verify(success, "bad xml in parse_core_data: {}",
                success.description());

    auto const& core_data_xml = d.child(XML_ISS_DATA).child(CORE_DATA);

    auto const& default_values_xml = core_data_xml.child(DEFAULT_VALUES);
    if (static_cast<bool>(default_values_xml)) {
      dv = parse_default_values(default_values_xml);
    }

    auto const& train_series_xml = core_data_xml.child(TRAIN_SERIES);
    if (static_cast<bool>(train_series_xml)) {
      auto const& train_series = rs::parse_train_series(train_series_xml);

      for (auto const& ts : train_series) {
        auto it = rs.train_series_.find(ts.get_key());
        utl::verify(it == std::end(rs.train_series_),
                    "Overwriting a train series!");

        rs.train_series_[ts.get_key()] = ts;
      }
    }
  }

  return {dv, rs};
}

void parse_xml_into_iss(std::string const& iss_xml, infrastructure_t& iss,
                        construction_materials& mats) {
  xml_document d;
  auto success = d.load_buffer(reinterpret_cast<void const*>(iss_xml.data()),
                               iss_xml.size());
  utl::verify(success, "bad xml: {}", success.description());

  for (auto const& xml_rp_station : d.child(XML_ISS_DATA)
                                        .child(RAIL_PLAN_STATIONS)
                                        .children(RAIL_PLAN_STATION)) {
    auto const id = static_cast<station::id>(iss.station_store_.size());
    iss.station_store_.emplace_back();
    iss.station_store_.back() =
        parse_iss_station(xml_rp_station, iss, mats, id);
    iss.stations_.emplace_back(iss.station_store_.back().get());
  }
}

std::pair<infrastructure_t, construction_materials> parse_iss(
    std::vector<utls::loaded_file> const& rail_plan_files) {
  utl::scoped_timer const station_timer("Parsing ISS Stations");

  std::pair<infrastructure_t, construction_materials> result;
  auto& iss = result.first;
  auto& mats = result.second;

  for (auto const& file : rail_plan_files) {
    uLOG(info) << "Station file: " << file.path_;
    parse_xml_into_iss(file.contents_, iss, mats);
  }

  return result;
}

auto get_layouted_positions(
    infrastructure_t const& iss, iss_files const& iss_files,
    soro::vector<gps> const& station_positions,
    soro::map<rail_plan_node_id, element_id> const& rp_id_to_element_id) {
  auto const layout = layout::get_layout(
      iss_files.rail_plan_files_, iss.stations_, iss.graph_.sections_,
      rp_id_to_element_id, iss.graph_.elements_.size());

  return layout::layout_to_gps(layout, iss.stations_, station_positions);
}

soro::vector<soro::string> get_full_station_names(
    infrastructure_t const& base_infra,
    regulatory_station_data const& regulatory_data) {
  return soro::to_vec(base_infra.stations_, [&](station::ptr s) {
    auto const it = regulatory_data.ds100_to_full_name_.find(s->ds100_);
    return it != std::end(regulatory_data.ds100_to_full_name_) ? it->second
                                                               : s->ds100_;
  });
}

infrastructure_t parse_iss(infrastructure_options const& options) {
  utl::scoped_timer const parse_timer("Parsing ISS");
  auto const iss_files = get_iss_files(options.infrastructure_path_);

  auto [iss, mats] = parse_iss(iss_files.rail_plan_files_);

  // ds100_to_station must be constructed before complete_borders
  iss.ds100_to_station_ = get_ds100_to_station(iss.stations_);

  complete_borders(iss);
  connect_nodes(iss.graph_);

  iss.element_to_station_ = get_element_to_station_map(iss);
  calculate_station_routes(iss, mats);

  auto const station_positions =
      parse_station_coords(options.gps_coord_path_, iss.ds100_to_station_);

  if (options.determine_layout_) {
    std::tie(iss.station_positions_, iss.element_positions_) =
        get_layouted_positions(iss, iss_files, station_positions,
                               mats.rp_id_to_element_id_);
  }

  auto const regulatory_station_data =
      parse_regulatory_stations(iss_files.regulatory_station_files_);
  iss.full_station_names_ =
      get_full_station_names(iss, regulatory_station_data);

  iss.station_route_graph_ =
      get_station_route_graph(iss.station_routes_, iss.graph_);

  std::tie(iss.defaults_, iss.rolling_stock_) =
      parse_core_data(iss_files.core_data_files_);

  if (options.determine_interlocking_) {
    iss.interlocking_ =
        get_interlocking_subsystem(iss, options.determine_conflicts_);
  }

  log_stats(iss);

  iss.source_ = options.infrastructure_path_.string();

  return std::move(iss);
}

// TODO(julian) print node types that can be omitted
// TODO(julian) print cycle detection in stations
// TODO(julian) print cycle detection in whole graph

}  // namespace soro::infra
