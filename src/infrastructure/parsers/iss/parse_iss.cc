#include "soro/infrastructure/parsers/iss/parse_iss.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "pugixml.hpp"

#include "utl/erase_duplicates.h"
#include "utl/logging.h"
#include "utl/timer.h"
#include "utl/verify.h"
#include "utl/zip.h"

#include "soro/base/soro_types.h"

#include "soro/utls/all.h"
#include "soro/utls/any.h"
#include "soro/utls/execute_if.h"
#include "soro/utls/narrow.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/std_wrapper/replace.h"
#include "soro/utls/std_wrapper/sort.h"
#include "soro/utls/string.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/critical_section.h"
#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/exclusion/get_exclusion.h"
#include "soro/infrastructure/graph/construction/connect_nodes.h"
#include "soro/infrastructure/graph/construction/create_element.h"
#include "soro/infrastructure/graph/construction/set_km.h"
#include "soro/infrastructure/graph/construction/set_line.h"
#include "soro/infrastructure/graph/construction/set_neighbour.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure_options.h"
#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/interlocking/get_interlocking.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/line.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/get_brake_tables.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/layout/parse_layout.h"
#include "soro/infrastructure/parsers/iss/parse_helpers.h"
#include "soro/infrastructure/parsers/iss/parse_station_route.h"
#include "soro/infrastructure/parsers/iss/parse_track_element.h"
#include "soro/infrastructure/path/length.h"
#include "soro/infrastructure/regulatory_data.h"
#include "soro/infrastructure/station/station.h"
#include "soro/infrastructure/station/station_route.h"
#include "soro/infrastructure/station/station_route_graph.h"

#include "soro/rolling_stock/parsers/iss/parse_rolling_stock.h"

namespace soro::infra {

using namespace pugi;
using namespace utl;

using namespace soro::si;
using namespace soro::rs;
using namespace soro::utls;

border parse_border(xml_node const& xml_rp_border, station::ptr const station,
                    element* border_element, line::id const line,
                    section::position const pos) {
  border b;

  b.neighbour_name_ = xml_rp_border.child_value(PARTNER_STATION);
  b.track_sign_ = std::stoi(xml_rp_border.child_value(TRACK_SIGN));
  b.line_ = line;
  b.station_ = station;
  b.element_ = border_element;
  b.pos_ = pos;

  return b;
}

switch_data get_switch_data(pugi::xml_node const& node) {
  std::string const name = node.child_value(NAME);

  auto const ui_identifier_xml = node.child(UI_IDENTIFIER);
  if (static_cast<bool>(ui_identifier_xml)) {
    return switch_data{.name_ = name,
                       .ui_identifier_ = ui_identifier_xml.child_value()};
  } else {
    return switch_data{.name_ = name, .ui_identifier_ = {}};
  }
}

using type_order_key = int16_t;

type_order_key get_type_order_key(xml_node const& node) {
  if (!node) {
    return std::numeric_limits<type_order_key>::max();
  }

  auto const type = get_type(node.name());

  if (is_section_element(type) ||
      type == any{type::META, type::PICTURE_POINT, type::INVALID}) {
    return std::numeric_limits<type_order_key>::max();
  }

  switch (str_hash(node.name())) {
    case str_hash(TUNNEL): return 10;
    case str_hash(SLOPE): return 20;
    case str_hash(ENTRY): return 30;

    case str_hash(HALT_PASSENGER_RIGHT_RISING): return 40;
    case str_hash(HALT_PASSENGER_RIGHT_FALLING): return 41;
    case str_hash(HALT_PASSENGER_LEFT_RISING): return 42;
    case str_hash(HALT_PASSENGER_LEFT_FALLING): return 43;
    case str_hash(HALT_FREIGHT_RISING): return 44;
    case str_hash(HALT_FREIGHT_FALLING): return 45;
    case str_hash(HALT_PASSENGER_RIGHT_LEFT_RISING): return 46;
    case str_hash(HALT_PASSENGER_RIGHT_LEFT_FALLING): return 47;
    case str_hash(HALT_PASSENGER_BOTH_RISING): return 48;
    case str_hash(HALT_PASSENGER_BOTH_FALLING): return 49;

    case str_hash(RUNTIME_CHECKPOINT): return 50;
    case str_hash(RUNTIME_CHECKPOINT_RISING): return 51;
    case str_hash(RUNTIME_CHECKPOINT_FALLING): return 52;

    case str_hash(MAIN_SIGNAL_RISING): return 60;
    case str_hash(MAIN_SIGNAL_FALLING): return 61;
    case str_hash(APPROACH_SIGNAL_RISING): return 62;
    case str_hash(APPROACH_SIGNAL_FALLING): return 63;

    case str_hash(PROTECTION_SIGNAL_RISING): return 70;
    case str_hash(PROTECTION_SIGNAL_FALLING): return 71;

    case str_hash(SIGNAL_EOTD_RISING): return 80;
    case str_hash(SIGNAL_EOTD_FALLING): return 81;
    case str_hash(ROUTE_EOTD_RISING): return 82;
    case str_hash(ROUTE_EOTD_FALLING): return 83;

    case str_hash(LZB_START_RISING): return 90;
    case str_hash(LZB_START_FALLING): return 91;

    case str_hash(LZB_END_RISING): return 92;
    case str_hash(LZB_END_FALLING): return 93;

    case str_hash(LZB_BLOCK_SIGN_RISING): return 94;
    case str_hash(LZB_BLOCK_SIGN_FALLING): return 95;

    case str_hash(LZB_EOTD_RISING): return 96;
    case str_hash(LZB_EOTD_FALLING): return 97;

    case str_hash(ETCS_START_RISING): return 100;
    case str_hash(ETCS_START_FALLING): return 101;

    case str_hash(ETCS_END_RISING): return 102;
    case str_hash(ETCS_END_FALLING): return 103;

    case str_hash(ETCS_BLOCK_SIGN_RISING): return 104;
    case str_hash(ETCS_BLOCK_SIGN_FALLING): return 105;

    case str_hash(SPEED_LIMIT):
    case str_hash(SPEED_LIMIT_RISING):
    case str_hash(SPEED_LIMIT_FALLING):
    case str_hash(SPEED_LIMIT_DIVERGENT_RISING):
    case str_hash(SPEED_LIMIT_DIVERGENT_FALLING):
    case str_hash(SPECIAL_SPEED_LIMIT_END_RISING):
    case str_hash(SPECIAL_SPEED_LIMIT_END_FALLING): return 110;

    case str_hash(FORCED_HALT_RISING): return 120;
    case str_hash(FORCED_HALT_FALLING): return 121;

    case str_hash(POINT_SPEED_RISING): return 130;
    case str_hash(POINT_SPEED_FALLING): return 131;

    case str_hash(BRAKE_PATH_RISING): return 140;
    case str_hash(BRAKE_PATH_FALLING): return 141;

    case str_hash(ELECTRIFICATION): return 250;

    case str_hash(ELECTRIFICATION_REPEATER): return 270;

    // undirected track elements part 2
    case str_hash(TRACK_NAME): return 300;
    case str_hash(LEVEL_CROSSING): return 310;

    default: {
      throw utl::fail("could not find type {} in get_type_order_key",
                      get_type_str(type));
    }
  }
}

template <typename Comparator, typename Id>
bool order_impl(Id const id1, xml_node const& n1, kilometrage const km1,
                Id const id2, xml_node const& n2, kilometrage const km2) {
  if (km1 != km2) {
    return Comparator{}(km1, km2);
  }

  auto const tok1 = get_type_order_key(n1);
  auto const tok2 = get_type_order_key(n2);

  if (tok1 != tok2) {
    return tok1 < tok2;
  }

  return id1 < id2;
}

template <typename Comparator>
bool element_order_from_nodes(xml_node const n1, xml_node const n2) {
  auto const km1 = parse_kilometrage(n1.child_value(KILOMETER_POINT));
  auto const km2 = parse_kilometrage(n2.child_value(KILOMETER_POINT));

  return order_impl<Comparator>(parse_rp_node_id(n1), n1, km1,
                                parse_rp_node_id(n2), n2, km2);
}

template <typename Comparator>
bool element_order(std::pair<element::ptr, xml_node> const& e1,
                   std::pair<element::ptr, xml_node> const& e2) {
  utls::expect(e1.first->is_track_element(), "e1 {} not a track element",
               e1.first->get_id());
  utls::expect(e2.first->is_track_element(), "e2 {} not a track element",
               e2.first->get_id());

  // nullptr here is okay, since we expect e1 and e2 are track elements
  auto const km1 = e1.first->km(nullptr);
  auto const km2 = e2.first->km(nullptr);

  return order_impl<Comparator>(e1.first->get_id(), e1.second, km1,
                                e2.first->get_id(), e2.second, km2);
}

section::id parse_section_into_network(xml_node const& xml_rp_section,
                                       dictionaries const& dicts,
                                       station& station, graph& network,
                                       construction_materials& mats) {
  auto const line_id =
      utls::parse_int<line::id>(xml_rp_section.child_value(LINE));
  auto const& section_start =
      xml_rp_section.child(RAIL_PLAN_NODE).children().begin();
  auto const& section_end =
      std::prev(xml_rp_section.child(RAIL_PLAN_NODE).children().end());

  auto const section_id = network.sections_.create_section();
  auto& sec = network.sections_[section_id];

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

  auto create_section_element = [&](auto const& node,
                                    section::position const pos) {
    auto const rp_id = parse_rp_node_id(node);
    auto const main_rp_id = get_main_rp_node_id(node);

    auto element = get_or_create_element(network, station, mats,
                                         get_type(node.name()), main_rp_id);
    if (rp_id != main_rp_id) {
      mats.to_element_id_.add(rp_id, element->get_id());
    }

    if (utls::equal(node.name(), SWITCH_START)) {
      network.element_data_[element->get_id()] = get_switch_data(node);
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
          parse_border(node, &station, element, line_id, pos));
    }

    return element;
  };

  auto start_element =
      create_section_element(*section_start, section::position::start);
  auto const start_km =
      parse_kilometrage(section_start->child_value(KILOMETER_POINT));
  set_km(*start_element, section_start->name(), start_km);
  set_line(*start_element, section_start->name(), line_id);
  network.sections_.add_section_id_to_element(
      *start_element, section_start->name(), section::position::start,
      section_id);

  auto end_element =
      create_section_element(*section_end, section::position::end);
  auto const end_km =
      parse_kilometrage(section_end->child_value(KILOMETER_POINT));
  set_km(*end_element, section_end->name(), end_km);
  set_line(*end_element, section_end->name(), line_id);
  network.sections_.add_section_id_to_element(
      *end_element, section_end->name(), section::position::end, section_id);

  auto const insert_meta_in_empty = [&](element* e1, element* e2,
                                        mileage_dir const dir,
                                        kilometrage const km) {
    utls::expect(utls::contains(e1->neighbours(), e2), "not neighbours");
    utls::expect(utls::contains(e2->neighbours(), e1), "not neighbours");

    auto meta =
        create_element_t<track_element>(network, station, mats, type::META,
                                        soro::optional<rail_plan_node_id>{});

    meta->as<track_element>().km_ = {km};
    meta->as<track_element>().lines_ = {line_id};
    meta->as<track_element>().neighbours_ = {e1, e2};
    meta->as<track_element>().dir_ = dir;

    network.sections_.add_section_id_to_element(
        *meta, "meta", section::position::middle, section_id);

    auto e1_neighbours = e1->neighbours();
    utls::replace(e1_neighbours, e2, meta);

    auto e2_neighbours = e2->neighbours();
    utls::replace(e2_neighbours, e1, meta);

    return meta;
  };

  auto const create_track_element =
      [&](xml_node const node, xml_node& prev_node, element*& prev_element,
          section::position& prev_pos, mileage_dir const dir) {
        auto const track_element = parse_track_element(
            node, dir, line_id, dicts, network, station, mats);

        set_neighbour(*prev_element, prev_node.name(), prev_pos, track_element,
                      dir);
        set_neighbour(*track_element, node.name(), section::position::middle,
                      prev_element, opposite(dir));

        network.sections_.add_section_id_to_element(
            *track_element, node.name(), section::position::middle, section_id);

        prev_element = track_element;
        prev_node = node;
        prev_pos = section::position::middle;

        return track_element;
      };

  std::vector<std::pair<element::ptr, xml_node>> all_track_elements;

  auto const insert_in_direction =
      [&](std::vector<xml_node> const& track_elements, xml_node& prev_node,
          element*& prev_element, section::position& prev_pos,
          mileage_dir const insert_dir) {
        for (auto const& node : track_elements) {

          auto const node_dir = get_track_element_direction(node);
          if (is_directed(node_dir) && node_dir != insert_dir) {
            continue;
          }

          auto const track_element = create_track_element(
              node, prev_node, prev_element, prev_pos, insert_dir);
          all_track_elements.emplace_back(track_element, node);
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
  auto prev_rising_pos = section::position::start;

  utls::sort(sorted_track_nodes, element_order_from_nodes<std::less<>>);
  insert_in_direction(sorted_track_nodes, prev_rising_rp_node,
                      prev_rising_element, prev_rising_pos,
                      mileage_dir::rising);
  set_neighbour(*prev_rising_element, prev_rising_rp_node.name(),
                prev_rising_pos, end_element, mileage_dir::rising);
  set_neighbour(*end_element, section_end->name(), section::position::end,
                prev_rising_element, mileage_dir::rising);

  if (prev_rising_element == start_element) {
    // section is empty in rising direction
    auto meta =
        insert_meta_in_empty(start_element, end_element, mileage_dir::rising,
                             start_km + (end_km - start_km) / 2);
    all_track_elements.emplace_back(meta, nullptr);
  }

  // scan the section in falling direction and add all falling and undirected
  // elements
  auto prev_falling_element = end_element;
  auto prev_falling_rp_node = *section_end;
  auto prev_falling_pos = section::position::end;

  utls::sort(sorted_track_nodes, element_order_from_nodes<std::greater<>>);
  insert_in_direction(sorted_track_nodes, prev_falling_rp_node,
                      prev_falling_element, prev_falling_pos,
                      mileage_dir::falling);
  set_neighbour(*prev_falling_element, prev_falling_rp_node.name(),
                prev_falling_pos, start_element, mileage_dir::falling);
  set_neighbour(*start_element, section_start->name(), section::position::start,
                prev_falling_element, mileage_dir::falling);

  if (prev_falling_element == end_element) {
    // section is empty in falling direction
    auto meta =
        insert_meta_in_empty(start_element, end_element, mileage_dir::falling,
                             start_km + (end_km - start_km) / 2);
    all_track_elements.emplace_back(meta, nullptr);
  }

  sec.rising_order_.emplace_back(start_element);
  // sort and copy track elements in rising order into the section
  utls::sort(all_track_elements, element_order<std::less<>>);
  for (auto const& p : all_track_elements) {
    sec.rising_order_.emplace_back(p.first);
  }
  sec.rising_order_.emplace_back(end_element);

  sec.falling_order_.emplace_back(end_element);
  // sort and copy track elements in falling order into the section
  utls::sort(all_track_elements, element_order<std::greater<>>);
  for (auto const& p : all_track_elements) {
    sec.falling_order_.emplace_back(p.first);
  }
  sec.falling_order_.emplace_back(start_element);

  return section_id;
}

std::vector<rail_plan_node_id> get_omitted_relevant_rp_nodes(
    intermediate_station_route const& isr) {
  type_set const INTERLOCKING_DISCERNING_TYPES = {type::MAIN_SIGNAL};

  std::vector<rail_plan_node_id> omitted_nodes;

  for (auto [rp_id, type] :
       utl::zip(isr.omitted_.nodes_, isr.omitted_.types_)) {
    if (INTERLOCKING_DISCERNING_TYPES.contains(type)) {
      omitted_nodes.emplace_back(rp_id);
    }
  }

  utls::sasserts([&] {
    auto const old_size = omitted_nodes.size();
    utl::erase_duplicates(omitted_nodes);
    utls::sassert(omitted_nodes.size() == old_size, "duplicates in omitted");
  });

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
  utl::scoped_timer const timer("deduplicating paths");

  auto const get_path = [&](intermediate_station_route const& sr,
                            node::ptr next_node, node::ptr const last_node) {
    uint32_t course_idx = 0;
    soro::vector<node::ptr> nodes;
    while (next_node != nullptr && next_node->id_ != last_node->id_) {
      nodes.push_back(next_node);

      auto const switch_element = next_node->element_->is_switch();

      if (next_node->branch_ == nullptr ||
          sr.course_[course_idx] == course_decision::STEM) {
        next_node = next_node->next_;
      } else {
        next_node = next_node->branch_;
      }

      course_idx += switch_element ? 1 : 0;
    }

    utl::verify(next_node != nullptr,
                "could not find path for station route {}", sr.name_);

    nodes.push_back(next_node);
    return nodes;
  };

  auto const get_start_node = [&infra](element::ptr const e) -> node::ptr {
    if (e->is(type::BORDER)) {
      using enum simple_element::nodes;
      using enum simple_element::direction;

      auto const& first_section =
          infra.graph_.sections_.get_section(e->get_id(), first);
      return e->node(first_section.is_empty_border_section() ? top : bot);
    }

    if (e->is_end_element()) {
      using enum end_element::nodes;
      using enum end_element::direction;

      auto const pos =
          infra.graph_.sections_.get_section_position(e->get_id(), oneway);
      return e->node(is_start(pos) ? top : bot);
    }

    if (e->is_track_element()) {
      return e->as<track_element>().node();
    }

    utls::unreachable(
        "station route must start with border, end or track_element");

    return nullptr;
  };

  auto const get_target_node = [&infra](element::ptr const e) -> node::ptr {
    if (e->is(type::BORDER)) {
      using enum simple_element::nodes;
      using enum simple_element::direction;

      auto const& first_section =
          infra.graph_.sections_.get_section(e->get_id(), first);
      return e->node(first_section.is_empty_border_section() ? bot : top);
    }

    if (e->is_end_element()) {
      using enum end_element::nodes;
      using enum end_element::direction;

      auto const pos =
          infra.graph_.sections_.get_section_position(e->get_id(), oneway);
      return e->node(is_start(pos) ? bot : top);
    }

    if (e->is_track_element()) {
      return e->as<track_element>().node();
    }

    utls::unreachable(
        "station route must end with border, end or track_element");

    return nullptr;
  };

  auto const get_main_signals = [&](intermediate_station_route const& isr,
                                    soro::vector<node::ptr> const& nodes) {
    std::set<node::ptr> omitted_main_signals;

    for (auto [rp_id, type] :
         utl::zip(isr.omitted_.nodes_, isr.omitted_.types_)) {
      if (type == type::MAIN_SIGNAL) {
        auto const element_id = mats.to_element_id_.first(rp_id);
        utls::sassert(mats.to_element_id_.second(rp_id) == element::invalid(),
                      "cannot have second id");
        omitted_main_signals.insert(
            infra.graph_.elements_[element_id]->as<track_element>().node());
      }
    }

    soro::vector<station_route::idx> main_signals;
    for (auto idx = 0U; idx < nodes.size(); ++idx) {
      auto const& node = nodes[idx];
      if (node->is(type::MAIN_SIGNAL) && !omitted_main_signals.contains(node)) {
        main_signals.emplace_back(static_cast<station_route::idx>(idx));
      }
    }

    return main_signals;
  };

  auto const get_etcs = [&](soro::vector<node::ptr> const& nodes)
      -> std::pair<soro::vector<station_route::idx>,
                   soro::vector<station_route::idx>> {
    soro::vector<station_route::idx> etcs_starts, etcs_ends;

    for (auto idx = 0U; idx < nodes.size(); ++idx) {
      if (nodes[idx]->is(type::ETCS_START)) {
        etcs_starts.emplace_back(utls::narrow<station_route::idx>(idx));
      }

      if (nodes[idx]->is(type::ETCS_END)) {
        etcs_ends.emplace_back(utls::narrow<station_route::idx>(idx));
      }
    }

    return {etcs_starts, etcs_ends};
  };

  auto const get_lzb = [&](soro::vector<node::ptr> const& nodes)
      -> std::pair<soro::vector<station_route::idx>,
                   soro::vector<station_route::idx>> {
    soro::vector<station_route::idx> lzb_starts, lzb_ends;

    for (auto idx = 0U; idx < nodes.size(); ++idx) {
      if (nodes[idx]->is(type::LZB_START)) {
        lzb_starts.emplace_back(utls::narrow<station_route::idx>(idx));
      }

      if (nodes[idx]->is(type::LZB_END)) {
        lzb_ends.emplace_back(utls::narrow<station_route::idx>(idx));
      }
    }

    return {lzb_starts, lzb_ends};
  };

  auto const& graph = infra.graph_;

  deduplicated_paths result;
  result.station_route_id_to_path_id_.resize(
      mats.intermediate_station_routes_.size());

  // this is just a helper data structure to help finding identical paths
  // it maps the following: element::id -> [{sr_id, path::ptr}]
  soro::vector_map<
      element::id,
      soro::vector<std::pair<station_route::id, station_route::path::ptr>>>
      start_elements_to_isr_paths(graph.elements_.size());

  for (auto const& i_sr : mats.intermediate_station_routes_) {
    auto start = graph.elements_[mats.to_element_id_.first(i_sr.start_)];
    auto end = graph.elements_[mats.to_element_id_.first(i_sr.end_)];

    utls::sassert(mats.to_element_id_.second(i_sr.start_) ==
                  element::invalid());
    utls::sassert(mats.to_element_id_.second(i_sr.end_) == element::invalid());

    // check if there already exists an identical path
    auto path_it = utls::find_if(
        start_elements_to_isr_paths[start->get_id()], [&](auto&& pair) {
          return same_path(
              mats.intermediate_station_routes_[as_val(pair.first)], i_sr);
        });

    if (path_it != std::end(start_elements_to_isr_paths[start->get_id()])) {
      // we found an identical path, use it
      result.station_route_id_to_path_id_[as_val(i_sr.id_)] = path_it->second;
      continue;
    }

    // create new path
    auto nodes = get_path(i_sr, get_start_node(start), get_target_node(end));
    auto main_signals = get_main_signals(i_sr, nodes);
    auto [etcs_starts, etcs_ends] = get_etcs(nodes);
    auto [lzb_starts, lzb_ends] = get_lzb(nodes);

    result.path_store_.emplace_back();
    result.path_store_.back() = soro::make_unique<station_route::path>(
        station_route::path{.start_ = start,
                            .end_ = end,
                            .course_ = i_sr.course_,
                            .nodes_ = std::move(nodes),
                            .main_signals_ = std::move(main_signals),
                            .etcs_starts_ = std::move(etcs_starts),
                            .etcs_ends_ = std::move(etcs_ends),
                            .lzb_starts_ = std::move(lzb_starts),
                            .lzb_ends_ = std::move(lzb_ends)});
    auto const path_ptr = result.path_store_.back().get();
    result.paths_.emplace_back(path_ptr);

    result.station_route_id_to_path_id_[as_val(i_sr.id_)] = path_ptr;
    start_elements_to_isr_paths[start->get_id()].emplace_back(i_sr.id_,
                                                              path_ptr);
  }

  return result;
}

void calculate_station_routes(infrastructure_t& infra,
                              construction_materials const& mats) {
  utl::scoped_timer const calc_timer("Calculating Station Routes");

  auto deduplicated_paths = get_station_route_paths(infra, mats);
  infra.station_route_path_store_ = std::move(deduplicated_paths.path_store_);
  infra.station_route_paths_ = std::move(deduplicated_paths.paths_);

  std::size_t through_routes = 0;
  std::size_t in_routes = 0;
  std::size_t out_routes = 0;

  infra.station_route_store_.reserve(mats.intermediate_station_routes_.size());
  for (auto const& i_sr : mats.intermediate_station_routes_) {
    infra.station_route_store_.emplace_back();
    sassert(infra.station_route_store_.size() == as_val(i_sr.id_) + 1,
            "Did not allocate enough space for the signal station route");

    infra.station_route_store_[as_val(i_sr.id_)] =
        soro::make_unique<station_route>();
    auto sr = infra.station_route_store_[as_val(i_sr.id_)].get();
    infra.station_routes_.emplace_back(sr);

    sr->id_ = static_cast<station_route::id>(i_sr.id_);
    sr->path_ =
        deduplicated_paths.station_route_id_to_path_id_[as_val(i_sr.id_)];
    sr->name_ = i_sr.name_;
    sr->station_ = i_sr.station_;
    sr->attributes_ = i_sr.attributes_;

    auto& station = infra.station_store_[as_val(i_sr.station_->id_)];

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

    if (auto next = sr->path_->nodes_.back()->next_; next != nullptr) {
      auto const next_station =
          infra.element_to_station_.at(next->element_->get_id());
      sr->to_station_ = next_station != sr->station_
                            ? station::optional_ptr(next_station)
                            : station::optional_ptr(std::nullopt);
    }

    if (auto inc = sr->path_->nodes_.front()->reverse_edges_; !inc.empty()) {
      auto const prev_station =
          infra.element_to_station_.at(inc.front()->element_->get_id());
      sr->from_station_ = prev_station != sr->station_
                              ? station::optional_ptr(prev_station)
                              : station::optional_ptr(std::nullopt);
    }

    auto const to_idx = [&](auto&& rp_id) {
      auto const element_ids = mats.to_element_id_[rp_id];

      utls::sassert(element::invalid() != utls::all{element_ids},
                    "no valid element id found");

      auto const it = utls::find_if(sr->nodes(), [&](auto&& n) {
        return n->element_->get_id() == any{element_ids};
      });

      utls::sassert(it != std::end(sr->nodes()), "could not find: {}", rp_id);

      return utls::narrow<station_route::idx>(
          std::distance(std::begin(sr->nodes()), it));
    };

    sr->passenger_halt_ = i_sr.passenger_halt_.transform(to_idx);
    sr->freight_halt_ = i_sr.freight_halt_.transform(to_idx);
    sr->runtime_checkpoint_ = i_sr.runtime_checkpoint_.transform(to_idx);

    sr->omitted_nodes_ = soro::to_vec(i_sr.omitted_.nodes_, to_idx);
    utls::sort(sr->omitted_nodes_);

    sr->extra_speed_limits_ =
        soro::to_vec(i_sr.speed_limits_, [&](auto&& i_spl) {
          return station_route::speed_limit{
              i_spl.spl_,
              utls::narrow<station_route::idx>(to_idx(i_spl.rp_id_))};
        });

    utls::sort(sr->extra_speed_limits_, [](auto const& spl1, auto const& spl2) {
      return spl1.idx_ < spl2.idx_;
    });

    sr->length_ = get_path_length_from_elements(sr->nodes());

    auto const alternative_to_speed_limit = [&](auto&& alternative) {
      utls::expects([&] {
        auto const e_id2 = mats.to_element_id_.second(alternative.node_id_);
        utls::sassert(e_id2 == element::invalid(), "only track elements");
      });

      auto const e_id = mats.to_element_id_.first(alternative.node_id_);

      // copy the speed limit from the infrastructure and modify the limit
      auto spl = infra.graph_.get_element_data<speed_limit>(e_id);
      spl.limit_ = alternative.speed_;

      return station_route::speed_limit{
          spl, utls::narrow<station_route::idx>(to_idx(alternative.node_id_))};
    };

    sr->alt_speed_limits_ =
        soro::to_vec(i_sr.alt_speed_limits_, alternative_to_speed_limit);

    utls::sort(sr->alt_speed_limits_,
               [](auto&& spl1, auto&& spl2) { return spl1.idx_ < spl2.idx_; });

    utls::sassert(sr->alt_speed_limits_.size() == i_sr.alt_speed_limits_.size(),
                  "did not account for every extra speed limit.");
    utls::sassert(sr->extra_speed_limits_.size() == i_sr.speed_limits_.size(),
                  "did not account for every extra speed limit.");
    utls::sassert(sr->omitted_nodes_.size() == i_sr.omitted_.nodes_.size(),
                  "did not account for every omitted node.");
    utls::sassert(
        sr->passenger_halt_.has_value() == i_sr.passenger_halt_.has_value(),
        "missed passenger halt");
    utls::sassert(
        sr->freight_halt_.has_value() == i_sr.freight_halt_.has_value(),
        "missed stop_mode halt");
    utls::sassert(sr->runtime_checkpoint_.has_value() ==
                      i_sr.runtime_checkpoint_.has_value(),
                  "missed runtime checkpoint");
  }

  // TODO(julian) refactor this into a separate function
  // fill element_to_station_routes_ map in every station
  for (auto& station : infra.station_store_) {
    for (auto const& [name, station_route] : station->station_routes_) {
      auto const first = station_route->nodes().front()->element_;
      auto it = station->element_to_routes_.find(first->get_id());
      if (it != std::end(station->element_to_routes_)) {
        it->second.push_back(station_route);
      } else {
        station->element_to_routes_[first->get_id()] = {station_route};
      }
    }
  }

  uLOG(info) << "parsed " << infra.station_routes_.size() << " station routes";
  uLOG(info) << "used " << infra.station_route_paths_.size() << " paths";
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
    auto const section_id = parse_section_into_network(
        section, iss.dictionaries_, *station, iss.graph_, mats);
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
        parse_station_route(sr_id, xml_sr, station.get()));
  }

  return station;
}

void complete_borders(infrastructure_t& iss) {
  expect(!iss.station_store_.empty(), "requires station already constructed");
  expect(!iss.ds100_to_station_.empty(), "requires DS100 to station mapping");

  for (auto& station : iss.station_store_) {
    for (auto& border : station->borders_) {
      utls::sassert(iss.ds100_to_station_.find(border.neighbour_name_) !=
                        std::end(iss.ds100_to_station_),
                    "could not find station {} when connecting borders",
                    border.neighbour_name_);

      border.neighbour_ = iss.ds100_to_station_[border.neighbour_name_];
    }
  }

  for (auto& from : iss.station_store_) {
    for (auto& from_border : from->borders_) {
      for (auto const& to_border : from_border.neighbour_->borders_) {
        if (to_border.neighbour_ != from.get() ||
            from_border.track_sign_ != to_border.track_sign_ ||
            from_border.line_ != to_border.line_) {
          continue;
        }

        from_border.neighbour_element_ = to_border.element_;
        connect_border(from_border, to_border, iss.graph_);
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

      auto const new_sec_id = iss.graph_.sections_.create_section();
      auto& new_sec = iss.graph_.sections_[new_sec_id];

      if (border.pos_ == section::position::end) {
        new_sec.rising_order_ = {border.element_, border.neighbour_element_};
        new_sec.falling_order_ = {border.neighbour_element_, border.element_};
      } else {
        new_sec.rising_order_ = {border.neighbour_element_, border.element_};
        new_sec.falling_order_ = {border.element_, border.neighbour_element_};
      }

      iss.graph_.sections_.add_section_id_to_element(
          *border.element_, BORDER, opposite(border.pos_), new_sec_id);
      iss.graph_.sections_.add_section_id_to_element(
          *border.neighbour_element_, BORDER, border.pos_, new_sec_id);

      finished_borders.insert(id_tuple);
    }
  }
}

auto get_element_to_station_map(infrastructure_t const& iss) {
  soro::map<element_id, station::ptr> element_to_station;

  for (auto const& station : iss.stations_) {
    for (auto const& element : station->elements_) {
      element_to_station[element->get_id()] = station;
    }
  }

  return element_to_station;
}

soro::map<soro::string, station::ptr> get_ds100_to_station(
    soro::vector_map<station::id, station::ptr> const& stations) {
  soro::map<soro::string, station::ptr> result;

  for (auto const& station : stations) {
    result[station->ds100_] = station;
  }

  return result;
}

void log_stats(infrastructure_t const& iss) {
  utl::scoped_timer const timer("logging statistics");

  uLOG(info) << "parsed ISS with " << iss.stations_.size() << " stations.";
  uLOG(info) << "parsed ISS with " << iss.graph_.elements_.size()
             << " elements and " << iss.graph_.nodes_.size() << " nodes.";

  std::size_t total_speed_limits = 0;
  std::size_t spl_with_length = 0;
  std::size_t spl_with_signal_poa = 0;
  std::size_t spl_with_here_poa = 0;
  std::size_t general_speed_limits = 0;
  std::size_t special_speed_limits = 0;
  std::size_t special_end_speed_limits = 0;
  std::size_t divergent_speed_limits = 0;

  std::map<soro::size_t, soro::size_t> approach_skips;

  for (auto const& data : iss.graph_.element_data_) {
    execute_if<speed_limit>(data, [&](auto&& spl) {
      ++total_speed_limits;
      spl_with_length += spl.length_.is_valid() ? 1 : 0;
      spl_with_signal_poa += spl.poa_ == speed_limit::poa::last_signal ? 1 : 0;
      spl_with_here_poa += spl.poa_ == speed_limit::poa::here ? 1 : 0;
      special_end_speed_limits += spl.ends_special() ? 1 : 0;
      general_speed_limits += spl.is_general() ? 1 : 0;
      special_speed_limits += spl.is_special() ? 1 : 0;
      divergent_speed_limits += spl.is_divergent() ? 1 : 0;
    });
  }

  uLOG(info) << "number of speed limits: " << total_speed_limits;
  uLOG(info) << "speed limits with length: " << spl_with_length;
  uLOG(info) << "speed limits with 'last signal' point of action: "
             << spl_with_signal_poa;
  uLOG(info) << "speed limits with 'here' point of action: "
             << spl_with_here_poa;
  uLOG(info) << "general speed limits: " << general_speed_limits;
  uLOG(info) << "special speed limits: " << special_speed_limits;
  uLOG(info) << "special end speed limits: " << special_end_speed_limits;
  uLOG(info) << "divergent speed limits: " << divergent_speed_limits;

  std::size_t total_section_elements = 0;
  std::size_t total_main_signals = 0;
  std::size_t total_simple_switches = 0;
  std::size_t total_crosses = 0;
  std::size_t total_cross_switches = 0;
  std::size_t switchables = 0;
  for (auto const& element : iss.graph_.elements_) {
    if (element->is(type::MAIN_SIGNAL)) {
      ++total_main_signals;

      auto const& ms = iss.graph_.get_element_data<main_signal>(element);
      approach_skips[ms.skip_approach_]++;
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

  uLOG(info) << "got " << total_main_signals << " main signals.";
  uLOG(info) << "got " << total_section_elements << " section elements.";
  uLOG(info) << "got " << total_simple_switches << " simple switches.";
  uLOG(info) << "got " << total_crosses << " crosses.";
  uLOG(info) << "got " << total_cross_switches << " cross switches.";
  uLOG(info) << "got " << switchables << " total switchables.";

  std::size_t total_variants = 0;
  for (auto const& ts : iss.rolling_stock_.train_series_) {
    total_variants += ts.second.variants_.size();
  }

  uLOG(info) << "got " << iss.rolling_stock_.train_series_.size()
             << " train series.";
  uLOG(info) << "got " << total_variants << " train series variants.";

  uLOG(info) << "default values:";
  uLOG(info) << "route form time: " << iss.defaults_.route_form_time_;
  uLOG(info) << "brake path length: " << iss.defaults_.brake_path_length_;
  uLOG(info) << "speed limit: " << iss.defaults_.stationary_speed_limit_;

  for (auto const& [skip, count] : approach_skips) {
    uLOG(info) << "skip approach " << skip << ": " << count;
  }
}

default_values parse_default_values(iss_files const& iss_files,
                                    dictionaries const& dicts) {

  auto const find_brake_path_with_length = [&](const char* const l) {
    for (brake_path bp{0}; bp < dicts.brake_path_.size(); ++bp) {
      if (str_contains(dicts.brake_path_.description(bp).data(), l)) return bp;
    }

    utls::unreachable("could not find brake path with length {}", l);

    return brake_path::invalid();
  };

  for (auto const& core_file_xml : iss_files.core_data_files_) {
    auto const core_xml = core_file_xml.child(XML_ISS_DATA).child(CORE_DATA);

    if (auto const dv_xml = core_xml.child(DEFAULT_VALUES); dv_xml) {

      default_values dv;

      dv.line_class_ = dv_xml.child_value(LINE_CLASS);
      dv.route_form_time_ =
          si::from_s(parse_fp<si::precision, replace_comma::ON>(
              dv_xml.child_value(ROUTE_FORM_TIME)));
      dv.brake_path_length_ =
          si::from_m(parse_fp<si::precision, replace_comma::ON>(
              dv_xml.child_value(BRAKE_PATH_LENGTH)));
      dv.stationary_speed_limit_ = si::from_km_h(
          parse_fp<si::precision>(dv_xml.child_value(STATIONARY_SPEED_LIMIT)));
      dv.brake_path_ =
          find_brake_path_with_length(dv_xml.child_value(BRAKE_PATH_LENGTH));

      return dv;
    }
  }

  utls::unreachable("no default values in iss found");

  return {};
}

void parse_xml_into_iss(xml_document const& iss_xml, infrastructure_t& iss,
                        construction_materials& mats) {
  for (auto const& xml_rp_station : iss_xml.child(XML_ISS_DATA)
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
    iss_files const& iss_files) {
  utl::scoped_timer const station_timer("parsing ISS stations");

  infrastructure_t infra;
  construction_materials mats;

  infra.dictionaries_ = get_dictionaries(iss_files);

  for (auto const& file : iss_files.rail_plan_files_) {
    parse_xml_into_iss(file, infra, mats);
  }

  utls::ensure(infra.graph_.node_to_rp_id_.size() == infra.graph_.nodes_.size(),
               "incorrect node to rp id mapping");
  utls::ensure(
      infra.graph_.element_to_rp_id_.size() == infra.graph_.elements_.size(),
      "incorrect element to rp id mapping");

  return {std::move(infra), std::move(mats)};
}

soro::vector_map<station::id, soro::string> get_full_station_names(
    infrastructure_t const& infra,
    regulatory_station_data const& regulatory_data) {
  soro::vector_map<station::id, soro::string> result;

  for (auto const& s : infra.stations_) {
    auto const it = regulatory_data.ds100_to_full_name_.find(s->ds100_);
    result.emplace_back(it != std::end(regulatory_data.ds100_to_full_name_)
                            ? it->second
                            : s->ds100_);
  }

  utls::ensure(result.size() == infra.stations_.size(),
               "ensure complete full name mapping");

  return result;
}

version parse_version(xml_document const& index_xml) {
  version v;

  auto const& version_xml =
      index_xml.child(XML_ISS_INDEX).child("SpurplandatenVersion");

  v.name_ = version_xml.child_value("Name");
  v.number_ =
      utls::parse_int<version::number>(version_xml.child_value("Nummer"));

  return v;
}

infrastructure_t parse_iss(infrastructure_options const& options) {
  utl::scoped_timer const parse_timer("parsing ISS from " +
                                      options.infrastructure_path_.string());

  iss_files const iss_files(options.infrastructure_path_);

  auto [infra, mats] = parse_iss(iss_files);

  infra.version_ = parse_version(iss_files.index_);

  // ds100_to_station must be constructed before complete_borders
  infra.ds100_to_station_ = get_ds100_to_station(infra.stations_);

  complete_borders(infra);
  connect_nodes(infra.graph_);

  infra.element_to_station_ = get_element_to_station_map(infra);
  calculate_station_routes(infra, mats);

  infra.station_route_graph_ =
      get_station_route_graph(infra.station_routes_, infra.graph_);

  if (options.layout_) {
    infra.layout_ = parse_layout(iss_files, infra, mats);
  }

  auto const regulatory_station_data =
      parse_regulatory_stations(iss_files.regulatory_station_files_);
  infra.full_station_names_ =
      get_full_station_names(infra, regulatory_station_data);

  infra.lines_ = parse_lines(iss_files.regulatory_line_files_);

  infra.defaults_ = parse_default_values(iss_files, infra.dictionaries_);

  infra.rolling_stock_ = parse_rolling_stock(iss_files, infra.dictionaries_);

  infra.critical_sections_ = get_critical_sections(infra.graph_);

  infra.brake_tables_ = get_brake_tables(iss_files, infra.dictionaries_);

  if (options.interlocking_) {
    infra.interlocking_ = get_interlocking(infra);
  }

  if (options.interlocking_ && options.exclusions_) {
    infra.exclusion_ =
        get_exclusion(infra, options.infrastructure_path_ / "exclusion_sets",
                      options.exclusion_sets_);
  }

  log_stats(infra);

  infra.source_ = options.infrastructure_path_.filename().string();

  return std::move(infra);
}

}  // namespace soro::infra
