#include "soro/infrastructure/parsers/iss/parse_iss.h"

#include <string>
#include <utility>

#include "pugixml.hpp"

#include "utl/enumerate.h"
#include "utl/erase_if.h"
#include "utl/get_or_create.h"
#include "utl/logging.h"
#include "utl/timer.h"
#include "utl/verify.h"

#include "soro/utls/execute_if.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/sort.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/exclusion/get_exclusion.h"
#include "soro/infrastructure/graph/graph_creation.h"
#include "soro/infrastructure/interlocking/get_interlocking.h"
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
  auto const start_km =
      parse_kilometrage(section_start.child_value(KILOMETER_POINT));
  auto const end_km =
      parse_kilometrage(section_end.child_value(KILOMETER_POINT));

  return abs(start_km - end_km);
}

border parse_border(xml_node const& xml_rp_border, element* border_element,
                    line::id const line, bool const is_start) {
  border b;

  b.neighbour_name_ = xml_rp_border.child_value(PARTNER_STATION);
  b.track_sign_ = std::stoi(xml_rp_border.child_value(TRACK_SIGN));
  b.line_ = line;
  b.element_ = border_element;
  b.low_border_ = is_start;

  return b;
}

switch_data get_switch_data(pugi::xml_node const& node) {
  std::string const name = node.child_value(NAME);

  auto const ui_identifier_xml = node.child("Oberflaechenbezeichner");
  if (static_cast<bool>(ui_identifier_xml)) {
    return switch_data{.name_ = name,
                       .ui_identifier_ = ui_identifier_xml.child_value()};
  } else {
    return switch_data{.name_ = name, .ui_identifier_ = {}};
  }
}

using type_order_key = int16_t;

constexpr type_order_key get_type_order_key(type const t, bool const rising) {
  switch (t) {
      // section elements
    case type::LINE_SWITCH:
    case type::KM_JUMP:
    case type::BORDER:
    case type::BUMPER:
    case type::SIMPLE_SWITCH:
    case type::CROSS:
    case type::TRACK_END:
      return std::numeric_limits<type_order_key>::max();

      // undirected track elements part 1
    case type::TUNNEL: return 0;
    case type::SLOPE: return 10;
    case type::ENTRY: return 20;
    case type::RUNTIME_CHECKPOINT_UNDIRECTED:
      return 100;

      // directed track elements
    case type::RUNTIME_CHECKPOINT: return rising ? 101 : 102;
    case type::HALT: return rising ? 110 : 111;
    case type::MAIN_SIGNAL: return rising ? 120 : 121;
    case type::APPROACH_SIGNAL: return rising ? 130 : 131;
    case type::PROTECTION_SIGNAL: return rising ? 140 : 141;
    case type::EOTD: return rising ? 150 : 151;
    case type::LZB_START: return rising ? 160 : 161;
    case type::LZB_BLOCK_SIGN: return rising ? 170 : 171;
    case type::LZB_END: return rising ? 180 : 181;
    case type::ETCS_START: return rising ? 190 : 191;
    case type::ETCS_BLOCK_SIGN: return rising ? 200 : 201;
    case type::ETCS_END: return rising ? 210 : 211;
    case type::SPEED_LIMIT: return rising ? 220 : 221;
    case type::FORCED_HALT: return rising ? 230 : 231;
    case type::POINT_SPEED: return rising ? 240 : 241;
    case type::BRAKE_PATH:
      return rising ? 250 : 251;

      // undirected track elements part 2
    case type::TRACK_NAME: return 300;
    case type::LEVEL_CROSSING:
      return 310;

      // meta
    case type::META:
      return 999;

      // invalid
    case type::INVALID: return std::numeric_limits<type_order_key>::max();
  }

  throw utl::fail("Could not find type {} in get_type_order_key",
                  get_type_str(t));
}

template <typename Comparator>
bool order_impl(kilometrage const km1, type const t1, bool const r1,
                element_id const id1, kilometrage const km2, type const t2,
                bool const r2, element_id const id2) {
  if (km1 != km2) {
    return Comparator{}(km1, km2);
  }

  auto const tok1 = get_type_order_key(t1, r1);
  auto const tok2 = get_type_order_key(t2, r2);

  if (tok1 != tok2) {
    return tok1 < tok2;
  }

  return id1 < id2;
}

template <typename Comparator>
bool element_order_from_nodes(xml_node const n1, xml_node const n2) {
  auto const km1 = parse_kilometrage(n1.child_value(KILOMETER_POINT));
  auto const km2 = parse_kilometrage(n2.child_value(KILOMETER_POINT));

  auto const r1 = has_rising_name(n1);
  auto const r2 = has_rising_name(n2);

  auto const t1 = get_type(n1.name());
  auto const t2 = get_type(n2.name());

  return order_impl<Comparator>(
      km1, t1, r1, static_cast<element_id>(parse_rp_node_id(n1)), km2, t2, r2,
      static_cast<element_id>(parse_rp_node_id(n2)));
}

template <typename Comparator>
bool element_order(element::ptr e1, element::ptr e2) {
  auto const km1 = e1->is_directed_track_element()
                       ? e1->as<track_element>().km_
                       : e1->as<undirected_track_element>().km_;
  auto const km2 = e2->is_directed_track_element()
                       ? e2->as<track_element>().km_
                       : e2->as<undirected_track_element>().km_;

  auto const r1 = e1->is_directed_track_element()
                      ? e1->as<track_element>().rising_
                      : e1->as<undirected_track_element>().rising_;
  auto const r2 = e2->is_directed_track_element()
                      ? e2->as<track_element>().rising_
                      : e2->as<undirected_track_element>().rising_;

  auto const t1 = e1->type();
  auto const t2 = e2->type();

  return order_impl<Comparator>(km1, t1, r1, e1->id(), km2, t2, r2, e2->id());
}

section::id parse_section_into_network(xml_node const& xml_rp_section,
                                       station& station, graph& network,
                                       construction_materials& mats) {
  auto const line_id =
      utls::parse_int<line::id>(xml_rp_section.child_value(LINE));
  auto const& section_start =
      xml_rp_section.child(RAIL_PLAN_NODE).children().begin();
  auto const& section_end =
      std::prev(xml_rp_section.child(RAIL_PLAN_NODE).children().end());

  auto const section_id = create_section(network);
  auto& sec = network.sections_[section_id];
  sec.length_ = get_section_length(*section_start, *section_end);
  sec.line_id_ = line_id;

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

    if (utls::equal(node.name(), SWITCH_START)) {
      network.element_data_[element->id()] = get_switch_data(node);
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
      station.borders_.push_back(
          parse_border(node, element, line_id, is_start));
    }

    return element;
  };

  auto start_element = emplace_into_network(*section_start, true);
  set_km_point_and_line(
      *start_element, section_start->name(),
      parse_kilometrage(section_start->child_value(KILOMETER_POINT)), line_id);
  network.element_id_to_section_ids_[start_element->id()].push_back(section_id);

  auto end_element = emplace_into_network(*section_end, false);
  set_km_point_and_line(
      *end_element, section_end->name(),
      parse_kilometrage(section_end->child_value(KILOMETER_POINT)), line_id);
  network.element_id_to_section_ids_[end_element->id()].push_back(section_id);

  std::map<xml_node, element*> undirected_track_elements;

  auto const insert_meta_in_empty = [&](element* e1, element* e2,
                                        bool const rising) {
    utls::sassert(utls::contains(e1->neighbours(), e2), "not neighbours");
    utls::sassert(utls::contains(e2->neighbours(), e1), "not neighbours");

    auto meta = create_element_t<track_element>(
        network, station, mats, type::META,
        std::numeric_limits<rail_plan_node_id>::max(), rising);

    meta->as<track_element>().km_ = e1->get_km(e2);
    meta->as<track_element>().line_ = line_id;
    meta->as<track_element>().ahead() = e2;
    meta->as<track_element>().behind() = e1;

    network.element_id_to_section_ids_[meta->id()].push_back(section_id);

    auto e1_neighbours = e1->neighbours();
    std::replace(std::begin(e1_neighbours), std::end(e1_neighbours), e2, meta);

    auto e2_neighbours = e2->neighbours();
    std::replace(std::begin(e2_neighbours), std::end(e2_neighbours), e1, meta);

    return meta;
  };

  auto const create_directed_track_element =
      [&](xml_node const node, xml_node& prev_node, element*& prev_element,
          bool const dir) {
        auto const type = get_type(node.name());

        auto const rising_element = has_rising_name(node);

        auto const track_element = parse_track_element(
            node, type, rising_element, line_id, network, station, mats);

        if (dir || prev_element->is_track_element()) {
          set_neighbour(*prev_element, prev_node.name(), track_element, true);
          set_neighbour(*track_element, node.name(), prev_element, false);
        } else {
          set_neighbour(*prev_element, prev_node.name(), track_element, false);
          set_neighbour(*track_element, node.name(), prev_element, false);
        }

        network.element_id_to_section_ids_[track_element->id()].push_back(
            section_id);

        prev_element = track_element;
        prev_node = node;

        return track_element;
      };

  auto const create_undirected_track_element =
      [&](xml_node const node, xml_node& prev_node, element*& prev_element,
          bool const dir) {
        auto const type = get_type(node.name());

        auto const track_element =
            utl::get_or_create(undirected_track_elements, node, [&]() {
              return parse_track_element(node, type, false, line_id, network,
                                         station, mats);
            });

        if (prev_element->is_section_element()) {
          set_neighbour(*prev_element, prev_node.name(), track_element, dir);
        } else {
          set_neighbour(*prev_element, prev_node.name(), track_element, true);
        }

        set_neighbour(*track_element, node.name(), prev_element, !dir);

        if (network.element_id_to_section_ids_[track_element->id()].empty()) {
          network.element_id_to_section_ids_[track_element->id()].push_back(
              section_id);
        }

        prev_element = track_element;
        prev_node = node;

        return track_element;
      };

  auto const create_track_element =
      [&](xml_node const node, xml_node& prev_node, element*& prev_element,
          bool const dir) {
        auto const type = get_type(node.name());
        return is_directed_track_element(type)
                   ? create_directed_track_element(node, prev_node,
                                                   prev_element, dir)
                   : create_undirected_track_element(node, prev_node,
                                                     prev_element, dir);
      };

  std::set<element::ptr> all_track_elements;

  auto const insert_in_direction =
      [&](std::vector<xml_node> const& track_elements, xml_node& prev_node,
          element*& prev_element, bool const direction) {
        for (auto const& node : track_elements) {

          if (is_directed_track_element(get_type(node.name())) &&
              has_rising_name(node) != direction) {
            continue;
          }

          auto const track_element =
              create_track_element(node, prev_node, prev_element, direction);
          all_track_elements.insert(track_element);
        }
      };

  // gather all track elements in one vector, we need to sort them in two
  // different orders later
  std::vector<xml_node> sorted_track_nodes;
  for (auto const node : xml_rp_section.child(RAIL_PLAN_NODE).children()) {
    auto const type = get_type(node.name());
    if (type == type::INVALID || !is_track_element(type)) {
      continue;
    }

    sorted_track_nodes.emplace_back(node);
  }

  // scan the section in rising direction and add all rising and undirected
  // elements
  auto prev_rising_element = start_element;
  auto prev_rising_rp_node = *section_start;

  utls::sort(sorted_track_nodes, element_order_from_nodes<std::less<>>);
  insert_in_direction(sorted_track_nodes, prev_rising_rp_node,
                      prev_rising_element, true);
  set_neighbour(*prev_rising_element, prev_rising_rp_node.name(), end_element,
                true);
  set_neighbour(*end_element, section_end->name(), prev_rising_element, true);

  if (prev_rising_element == start_element) {
    // section is empty in rising direction
    auto meta = insert_meta_in_empty(start_element, end_element, true);
    all_track_elements.insert(meta);
  }

  // scan the section in falling direction and add all falling and undirected
  // elements
  auto prev_falling_element = end_element;
  auto prev_falling_rp_node = *section_end;

  utls::sort(sorted_track_nodes, element_order_from_nodes<std::greater<>>);
  insert_in_direction(sorted_track_nodes, prev_falling_rp_node,
                      prev_falling_element, false);
  set_neighbour(*start_element, section_start->name(), prev_falling_element,
                false);

  if (prev_falling_element->is_track_element()) {
    set_neighbour(*prev_falling_element, prev_falling_rp_node.name(),
                  start_element, true);
  } else {
    set_neighbour(*prev_falling_element, prev_falling_rp_node.name(),
                  start_element, false);
  }

  if (prev_falling_element == end_element) {
    // section is empty in falling direction
    auto meta = insert_meta_in_empty(end_element, start_element, false);
    all_track_elements.insert(meta);
  }

  // don't sort the first section element, since when a section has length 0
  // we don't want the order to be
  // [section element, section element, track element, ...]
  sec.rising_order_.emplace_back(start_element);
  sec.rising_order_.insert(std::end(sec.rising_order_),
                           std::begin(all_track_elements),
                           std::end(all_track_elements));
  std::sort(std::begin(sec.rising_order_) + 1, std::end(sec.rising_order_),
            element_order<std::less<>>);
  sec.rising_order_.emplace_back(end_element);

  sec.falling_order_.emplace_back(end_element);
  sec.falling_order_.insert(std::end(sec.falling_order_),
                            std::begin(all_track_elements),
                            std::end(all_track_elements));
  std::sort(std::begin(sec.falling_order_) + 1, std::end(sec.falling_order_),
            element_order<std::greater<>>);
  sec.falling_order_.emplace_back(start_element);

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
  utl::scoped_timer const timer("Deduplicating Paths");

  auto const get_path = [&](intermediate_station_route const& sr,
                            node::ptr next_node, auto const last_node_id) {
    uint32_t course_idx = 0;
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
                                    soro::vector<node::ptr> const& nodes) {
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
    for (auto idx = 0U; idx < nodes.size(); ++idx) {
      auto const& node = nodes[idx];
      if (node->is(type::MAIN_SIGNAL) && !omitted_main_signals.contains(node)) {
        main_signals.emplace_back(static_cast<node::idx>(idx));
      }
    }

    return main_signals;
  };

  auto const get_etcs = [&](soro::vector<node::ptr> const& nodes)
      -> std::pair<soro::vector<node::idx>, soro::vector<node::idx>> {
    soro::vector<node::idx> etcs_starts, etcs_ends;

    for (node::idx idx = 0; idx < static_cast<node::idx>(nodes.size()); ++idx) {
      auto const& node = nodes[idx];
      if (node->is(type::ETCS_START)) {
        etcs_starts.emplace_back(idx);
      }

      if (node->is(type::ETCS_END)) {
        etcs_ends.emplace_back(idx);
      }
    }

    return {etcs_starts, etcs_ends};
  };

  auto const& graph = infra.graph_;

  deduplicated_paths result;
  result.station_route_id_to_path_id_.resize(
      mats.intermediate_station_routes_.size());

  // this is just a helper data structure to help finding identical paths
  // it maps the following: element::id -> [{sr_id, path::ptr}]
  soro::vector<soro::vector<std::pair<element_id, station_route::path::ptr>>>
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
    auto [etcs_starts, etcs_ends] = get_etcs(nodes);

    result.path_store_.emplace_back();
    result.path_store_.back() = soro::make_unique<station_route::path>(
        station_route::path{.start_ = start,
                            .end_ = end,
                            .course_ = i_sr.course_,
                            .nodes_ = std::move(nodes),
                            .main_signals_ = std::move(main_signals),
                            .etcs_starts_ = std::move(etcs_starts),
                            .etcs_ends_ = std::move(etcs_ends)});
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
      sr->to_station_ = next_station != sr->station_
                            ? station::optional_ptr(next_station)
                            : station::optional_ptr(std::nullopt);
    }

    if (auto inc = sr->path_->nodes_.front()->reverse_edges_; !inc.empty()) {
      auto const prev_station =
          infra.element_to_station_.at(inc.front()->element_->id());
      sr->from_station_ = prev_station != sr->station_
                              ? station::optional_ptr(prev_station)
                              : station::optional_ptr(std::nullopt);
    }

    auto const rp_id_to_element = [&](auto&& rp_id) -> element::ptr {
      return rp_id == INVALID_RP_NODE_ID
                 ? nullptr
                 : graph.elements_[mats.rp_id_to_element_id_.at(rp_id)];
    };

    auto const passenger_element = rp_id_to_element(i_sr.rp_passenger_halt_);
    auto const freight_element = rp_id_to_element(i_sr.rp_freight_halt_);
    auto const runtime_check_e = rp_id_to_element(i_sr.rp_runtime_checkpoint_);

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

      if (node->element_ == runtime_check_e) {
        sr->runtime_checkpoint_ = node::optional_idx(idx);
      }

      if (node->element_ == passenger_element) {
        sr->passenger_halt_ = node::optional_idx(idx);
      }

      if (node->element_ == freight_element) {
        sr->freight_halt_ = node::optional_idx(idx);
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
    auto const sr_id = static_cast<station_route::id>(
        mats.intermediate_station_routes_.size());

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

#if defined(SERIALIZE) && !defined(USE_CISTA_RAW)
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
        new_sec.rising_order_ = {border.neighbour_element_, border.element_};
        new_sec.falling_order_ = {border.element_, border.neighbour_element_};
      } else {
        new_sec.rising_order_ = {border.element_, border.neighbour_element_};
        new_sec.falling_order_ = {border.neighbour_element_, border.element_};
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
  soro::map<soro::string, station::ptr> result;

  for (auto const& station : stations) {
    result[station->ds100_] = station;
  }

  return result;
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

  std::size_t total_variants = 0;
  for (auto const& ts : iss.rolling_stock_.train_series_) {
    total_variants += ts.second.variants_.size();
  }

  uLOG(info) << "Got " << iss.rolling_stock_.train_series_.size()
             << " train series.";
  uLOG(info) << "Got " << total_variants << " train series variants.";

  uLOG(info) << "Default values:";
  uLOG(info) << "Route form time: " << iss.defaults_.route_form_time_;
  uLOG(info) << "Brake path length: " << iss.defaults_.brake_path_length_;
  uLOG(info) << "Speed limit: " << iss.defaults_.stationary_speed_limit_;
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
    auto success = d.load_buffer(
        reinterpret_cast<void const*>(core_file.data()), core_file.size());
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

void parse_xml_into_iss(utls::loaded_file const& iss_xml, infrastructure_t& iss,
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
    parse_xml_into_iss(file, iss, mats);
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
    if (it != std::end(regulatory_data.ds100_to_full_name_)) {
      return it->second;
    } else {
      return s->ds100_;
    }
  });
}

version parse_version(utls::loaded_file const& index) {
  version v;

  xml_document d;
  auto success =
      d.load_buffer(reinterpret_cast<void const*>(index.data()), index.size());
  utl::verify(success, "bad index xml: {}", success.description());

  auto const& version_xml =
      d.child(XML_ISS_INDEX).child("SpurplandatenVersion");

  v.name_ = version_xml.child_value("Name");
  v.number_ =
      utls::parse_int<version::number>(version_xml.child_value("Nummer"));

  return v;
}

infrastructure_t parse_iss(infrastructure_options const& options) {
  utl::scoped_timer const parse_timer("Parsing ISS");
  auto const iss_files = get_iss_files(options.infrastructure_path_);

  auto [iss, mats] = parse_iss(iss_files.rail_plan_files_);

  iss.version_ = parse_version(iss_files.index_);

  // ds100_to_station must be constructed before complete_borders
  iss.ds100_to_station_ = get_ds100_to_station(iss.stations_);

  complete_borders(iss);
  connect_nodes(iss.graph_);

  iss.element_to_station_ = get_element_to_station_map(iss);
  calculate_station_routes(iss, mats);

  if (options.layout_) {
    auto const station_positions =
        parse_station_coords(options.gps_coord_path_, iss.ds100_to_station_);

    std::tie(iss.station_positions_, iss.element_positions_) =
        get_layouted_positions(iss, iss_files, station_positions,
                               mats.rp_id_to_element_id_);
  }

  auto const regulatory_station_data =
      parse_regulatory_stations(iss_files.regulatory_station_files_);
  iss.full_station_names_ =
      get_full_station_names(iss, regulatory_station_data);

  iss.lines_ = parse_lines(iss_files.regulatory_line_files_);

  iss.station_route_graph_ =
      get_station_route_graph(iss.station_routes_, iss.graph_);

  std::tie(iss.defaults_, iss.rolling_stock_) =
      parse_core_data(iss_files.core_data_files_);

  if (options.interlocking_) {
    iss.interlocking_ = get_interlocking(iss);
  }

  if (options.interlocking_ && options.exclusions_) {
    iss.exclusion_ =
        get_exclusion(iss, options.infrastructure_path_ / "exclusion_sets",
                      options.exclusion_elements_, options.exclusion_graph_);
  }

  log_stats(iss);

  iss.source_ = options.infrastructure_path_.filename().string();

  return std::move(iss);
}

}  // namespace soro::infra
