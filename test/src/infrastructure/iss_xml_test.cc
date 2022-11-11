#include "doctest/doctest.h"

#include <cstddef>
#include <string>

#include "cista/serialization.h"
#include "fmt/format.h"

#include "utl/enumerate.h"
#include "utl/pipes.h"
#include "utl/to_vec.h"

#include "soro/utls/container/constexpr_map.h"
#include "soro/utls/execute_if.h"
#include "soro/utls/graph/traversal.h"

#include "soro/infrastructure/infrastructure.h"

#include "test/file_paths.h"

#if defined(SERIALIZE)
#include "cista/serialization.h"
#endif

using namespace soro;
using namespace soro::si;
using namespace soro::utls;
using namespace soro::infra;

constexpr std::array<std::pair<type, size_t>,
                     static_cast<size_t>(type::INVALID)>
    expected_edges_arr{{
        {type::BORDER, 1},
        {type::BUMPER, 1},
        {type::TRACK_END, 1},
        {type::KM_JUMP, 2},
        {type::LINE_SWITCH, 2},
        {type::SIMPLE_SWITCH, 3},
        {type::CROSS, 4},
        {type::MAIN_SIGNAL, 1},
        {type::PROTECTION_SIGNAL, 1},
        {type::APPROACH_SIGNAL, 1},
        {type::RUNTIME_CHECKPOINT, 1},
        {type::EOTD, 1},
        {type::SPEED_LIMIT, 1},
        {type::TUNNEL, 1},
        {type::CTC, 1},
        {type::SLOPE, 1},
        {type::HALT, 1},
    }};

static_assert(expected_edges_arr.size() == static_cast<size_t>(type::INVALID));

auto constexpr expected_edges =
    utls::constexpr_map<type, size_t, expected_edges_arr.size()>{
        expected_edges_arr};

void check_switch(simple_switch const& ss, soro::string const& station_name) {

  using namespace std::string_literals;
  using namespace std::literals;

  auto in_station = fmt::format("in station {}", station_name);

  auto const get_node = [](element const& e, element_id const id) {
    for (auto const& n : e.nodes()) {
      if (n->next_node_ != nullptr && n->next_node_->element_->id() == id) {
        return std::pair(n, false);
      }

      if (n->branch_node_ != nullptr && n->branch_node_->element_->id() == id) {
        return std::pair(n, true);
      }
    }

    throw utl::fail(
        "Could not find the correct node from the neighbour to the switch!");
  };

  {  // check start -> stem and start -> branch
    auto const& [node, take_branch] =
        get_node((ss.rising_ ? *ss.rising_start_neighbour()
                             : *ss.falling_start_neighbour()),
                 ss.id_);

    auto const stem_id = (take_branch ? node->branch_node_ : node->next_node_)
                             ->next_node_->element_->id();

    CHECK_MESSAGE(stem_id == (ss.stem_rising_ ? ss.falling_stem_neighbour()
                                              : ss.rising_stem_neighbour())
                                 ->id(),
                  in_station);

    auto const branch_id = (take_branch ? node->branch_node_ : node->next_node_)
                               ->branch_node_->element_->id();
    CHECK_MESSAGE(
        branch_id == (ss.branch_rising_ ? ss.falling_branch_neighbour()
                                        : ss.rising_branch_neighbour())
                         ->id(),
        in_station);
  }

  {  // check stem -> start
    auto const& [node, take_branch] =
        get_node((ss.stem_rising_ ? *ss.rising_stem_neighbour()
                                  : *ss.falling_stem_neighbour()),
                 ss.id_);

    auto const start_id = (take_branch ? node->branch_node_ : node->next_node_)
                              ->next_node_->element_->id();

    CHECK_MESSAGE(start_id == (ss.rising_ ? ss.falling_start_neighbour()
                                          : ss.rising_start_neighbour())
                                  ->id(),
                  in_station);
  }

  {  // check branch -> start
    auto const& [node, take_branch] =
        get_node((ss.branch_rising_ ? *ss.rising_branch_neighbour()
                                    : *ss.falling_branch_neighbour()),
                 ss.id_);

    auto const start_id = (take_branch ? node->branch_node_ : node->next_node_)
                              ->next_node_->element_->id();

    CHECK_MESSAGE(start_id == (ss.rising_ ? ss.falling_start_neighbour()
                                          : ss.rising_start_neighbour())
                                  ->id(),
                  in_station);
  }
}

void check_cross(cross const& cross, soro::string const& station_name) {
  auto const in_station = fmt::format("in station {}", station_name);

  auto const get_cross_node = [](element const& e, element_id const id) {
    for (auto const& n : e.nodes()) {
      if (n->next_node_ != nullptr && n->next_node_->element_->id() == id) {
        return n->next_node_;
      }

      if (n->branch_node_ != nullptr && n->branch_node_->element_->id() == id) {
        return n->branch_node_;
      }
    }

    throw utl::fail(
        "Could not find the correct node from the neighbour to the switch!");
  };

  {  // check start_left -> end_left and start_left -> end_right
    auto const cross_node =
        get_cross_node((cross.rising_ ? *cross.rising_start_left()
                                      : *cross.falling_start_left()),
                       cross.id_);

    auto const end_left_id = cross_node->next_node_->element_->id();
    CHECK_MESSAGE(
        end_left_id == (cross.end_left_rising_ ? cross.falling_end_left()
                                               : cross.rising_end_left())
                           ->id(),
        in_station);

    if (cross.start_left_end_right_arc_) {
      auto const end_right_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE(
          end_right_id == (cross.end_right_rising_ ? cross.falling_end_right()
                                                   : cross.rising_end_right())
                              ->id(),
          in_station);
    }
  }

  {  // check end_left -> start_left and end_left -> start_right
    auto const cross_node =
        get_cross_node((cross.end_left_rising_ ? *cross.rising_end_left()
                                               : *cross.falling_end_left()),
                       cross.id_);

    auto const start_left_id = cross_node->next_node_->element_->id();
    CHECK_MESSAGE(start_left_id == (cross.rising_ ? cross.falling_start_left()
                                                  : cross.rising_start_left())
                                       ->id(),
                  in_station);

    if (cross.start_right_end_left_arc_) {
      auto const start_right_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE(start_right_id == (cross.start_right_rising_
                                           ? cross.falling_start_right()
                                           : cross.rising_start_right())
                                          ->id(),
                    in_station);
    }
  }

  {  // check start_right -> end_right and start_right -> end_left
    auto const cross_node = get_cross_node(
        (cross.start_right_rising_ ? *cross.rising_start_right()
                                   : *cross.falling_start_right()),
        cross.id_);

    auto const end_right_id = cross_node->next_node_->element_->id();
    CHECK_MESSAGE(
        end_right_id == (cross.end_right_rising_ ? cross.falling_end_right()
                                                 : cross.rising_end_right())
                            ->id(),
        in_station);

    if (cross.start_right_end_left_arc_) {
      auto const end_left_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE(
          end_left_id == (cross.end_left_rising_ ? cross.falling_end_left()
                                                 : cross.rising_end_left())
                             ->id(),
          in_station);
    }
  }

  {  // check end_right -> start_right and end_right -> start_left
    auto const cross_node =
        get_cross_node((cross.end_right_rising_ ? *cross.rising_end_right()
                                                : *cross.falling_end_right()),
                       cross.id_);

    auto const start_right_id = cross_node->next_node_->element_->id();
    CHECK_MESSAGE(start_right_id == (cross.start_right_rising_
                                         ? cross.falling_start_right()
                                         : cross.rising_start_right())
                                        ->id(),
                  in_station);

    if (cross.start_left_end_right_arc_) {
      auto const start_left_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE(start_left_id == (cross.rising_ ? cross.falling_start_left()
                                                    : cross.rising_start_left())
                                         ->id(),
                    in_station);
    }
  }
}

void check_outgoing(element const& element, station const& station) {
  size_t outgoing = 0;
  for (auto const& node_ptr : element.nodes()) {
    // Count outgoing edges
    outgoing += node_ptr->next_node_ != nullptr ? 1 : 0;
    outgoing += node_ptr->branch_node_ != nullptr ? 1 : 0;
  }
  auto expected = expected_edges.at(element.type());

  // Crosses can also be single or double cross switches, adjust expected
  if (element.is(type::CROSS)) {
    if (element.as<cross>().start_left_end_right_arc_) {
      expected += 2;
    }

    if (element.as<cross>().start_right_end_left_arc_) {
      expected += 2;
    }
  }

  // Borders can connect to other stations, adjust expected
  if (element.is(type::BORDER)) {
    bool realized_border = false;
    for (auto const& border : station.borders_) {
      if (border.element_->id() == element.id()) {
        realized_border = true;
        break;
      }
    }

    if (realized_border) {
      expected += 1;
    }
  }

  CHECK_MESSAGE(expected == outgoing,
                fmt::format("Expected outgoing edges differ from actual "
                            "outgoing edges for element with id: {}.",
                            element.id()));
}

void check_incoming(infrastructure const& infra,
                    std::vector<size_t> const& incoming_edges,
                    std::vector<size_t> const& expected_incoming) {
  for (auto const& station : infra->stations_) {
    for (auto const& element : station->elements_) {

      auto const incoming = incoming_edges[element->id()];
      auto expected = expected_edges.at(element->type());

      if (element->is(type::CROSS)) {
        if (element->as<cross>().start_left_end_right_arc_) {
          expected += 2;
        }

        if (element->as<cross>().start_right_end_left_arc_) {
          expected += 2;
        }
      }

      // Borders can connect to other stations, adjust expected
      if (element->is(type::BORDER)) {
        bool realized_border = false;
        for (auto const& border : station->borders_) {
          if (border.element_->id() == element->id()) {
            realized_border = true;
            break;
          }
        }

        if (realized_border) {
          expected += 1;
        }

        expected += expected_incoming[element->id()];

        auto const message = fmt::format(
            "Expected incoming edges differ from actual incoming edges for "
            "element with id {} and type {} in station {}.",
            element->id(), element->get_type_str(), station->ds100_);
        CHECK_MESSAGE(expected == incoming, message);
      }
    }
  }
}

void check_neighbours(element const& e) {
  for (auto const& neigh : e.neighbours()) {
    CHECK_NE(neigh, nullptr);
  }
}

void check_network(infrastructure const& infra) {
  size_t total_elements = 0;
  std::vector<size_t> incoming_edges(infra->graph_.elements_.size(), 0);
  std::vector<size_t> expected_incoming(infra->graph_.elements_.size(), 0);

  for (auto const& station : infra->stations_) {
    total_elements += station->elements_.size();

    for (auto const& element : station->elements_) {
      check_neighbours(*element);

      check_outgoing(*element, *station);

      if (element->is(type::SIMPLE_SWITCH)) {
        check_switch(element->as<simple_switch>(), station->ds100_);
      }

      if (element->is(type::CROSS)) {
        auto const& cross = element->as<struct cross>();

        if (cross.start_left_end_right_arc_) {
          ++(expected_incoming[(cross.rising_ ? cross.falling_start_left()
                                              : cross.rising_start_left())
                                   ->id()]);
          ++(expected_incoming[(cross.end_right_rising_
                                    ? cross.falling_end_right()
                                    : cross.rising_end_right())
                                   ->id()]);
        }

        if (cross.start_right_end_left_arc_) {
          ++(expected_incoming[(cross.start_right_rising_
                                    ? cross.falling_start_right()
                                    : cross.rising_start_right())
                                   ->id()]);
          ++(expected_incoming[(cross.end_left_rising_
                                    ? cross.falling_end_left()
                                    : cross.rising_end_left())
                                   ->id()]);
        }

        check_cross(cross, station->ds100_);
      }

      for (auto const& node_ptr : element->nodes()) {
        // Check: if there is a -> b, then b -> a should not exist
        auto const cycle_message =
            fmt::format("Found cycle in element with id {} in station {}",
                        element->id(), station->ds100_);
        if (node_ptr->next_node_ != nullptr) {
          ++(incoming_edges[node_ptr->next_node_->element_->id()]);

          CHECK_MESSAGE(node_ptr->next_node_->next_node_ != node_ptr,
                        cycle_message);
          CHECK_MESSAGE(node_ptr->next_node_->branch_node_ != node_ptr,
                        cycle_message);
        }

        if (node_ptr->branch_node_ != nullptr) {
          ++(incoming_edges[node_ptr->branch_node_->element_->id()]);

          CHECK_MESSAGE(node_ptr->branch_node_->next_node_ != node_ptr,
                        cycle_message);
          CHECK_MESSAGE(node_ptr->branch_node_->branch_node_ != node_ptr,
                        cycle_message);
        }
      }
    }
  }

  check_incoming(infra, incoming_edges, expected_incoming);

  CHECK(total_elements == infra->graph_.elements_.size());
}

void check_station_routes(infrastructure const& infra) {
  for (auto const& sr : infra->station_routes_) {
    CHECK(station_route::valid(sr->id_));

    CHECK(!sr->nodes().empty());
    CHECK(!sr->name_.empty());

    CHECK_NE(sr->start_element_, nullptr);
    CHECK_NE(sr->end_element_, nullptr);

    REQUIRE_NE(sr->station_, nullptr);

    if (sr->nodes().back()->next_node_ != nullptr) {
      CHECK_NE(sr->to_, nullptr);
    }

    if (!sr->nodes().front()->reverse_edges_.empty()) {
      CHECK_NE(sr->from_, nullptr);
    }

    CHECK(si::valid(sr->length_));
  }
}

void check_station_route_graph(infrastructure const& infra) {
  for (auto const& sr : infra->station_routes_) {
    auto const& succs = infra->station_route_graph_.successors_[sr->id_];

    for (auto const& succ : succs) {
      CHECK_MESSAGE(succ->id_ != sr->id_,
                    "Station route can't be its own successor!");
    }
  }
}

template <typename Container>
void check_continuous_ascending_ids(Container const& c) {
  for (auto [e1, e2] : utl::pairwise(c)) {
    if constexpr (soro::is_pointer<std::remove_reference_t<decltype(e1)>>()) {
      CHECK(e1->id_ + 1 == e2->id_);
    } else {
      CHECK(e1.id_ + 1 == e2.id_);
    }
  }
}

void check_ascending_ids(infrastructure const& infra) {
  check_continuous_ascending_ids(infra->stations_);
  check_continuous_ascending_ids(infra->station_routes_);
}

void check_speed_limit_values(infrastructure const& infra) {
  // if a special speed limit band is ending, it does not need a speed value
  for (auto const& data : infra->graph_.element_data_) {
    execute_if<speed_limit>(data, [](auto&& spl) {
      if (spl.type_ != speed_limit::type::END_SPECIAL) {
        CHECK(valid(spl.limit_));
      }
    });
  }
}

void check_signal_station_routes(infrastructure const& infra) {
  for (auto const& ssr : infra->interlocking_.interlocking_routes_) {
    auto const& first_node = ssr->nodes().front();
    auto const& last_node = ssr->nodes().back();

    auto const first_node_is_valid =
        interlocking_route::valid_ends().contains(first_node->type());
    auto const last_node_is_valid =
        interlocking_route::valid_ends().contains(last_node->type());

    CHECK_MESSAGE(first_node_is_valid,
                  fmt::format("First element's type must be from the list of "
                              "valid elements, but was {}.",
                              first_node->element_->get_type_str()));

    CHECK_MESSAGE(last_node_is_valid,
                  fmt::format("Last element's type must be from the list of "
                              "valid elements, but was {}.",
                              last_node->element_->get_type_str()));
  }
}

void check_signal_station_route_count(infrastructure const& infra) {
  soro::size_type inner_sr_count = 0;
  soro::size_type path_sr_count = 0;

  for (auto const& sr : infra->station_routes_) {
    if (sr->main_signals_.empty()) {
      continue;
    }

    soro::size_type reachable_ms = 0;

    auto const& handle_node = [&](auto&& sr_ptr, auto&&) {
      if (sr_ptr != sr && !sr_ptr->main_signals_.empty()) {
        ++reachable_ms;
      }

      return false;
    };

    auto const& get_neighbours = [&](auto&& sr_ptr) {
      if (sr_ptr == sr || sr_ptr->main_signals_.empty()) {
        return infra->station_route_graph_.successors_[sr_ptr->id_];
      } else {
        return decltype(infra->station_route_graph_.successors_.front()){};
      }
    };

    utls::dfs(sr, handle_node, get_neighbours);

    path_sr_count += reachable_ms;

    if (sr->main_signals_.size() > 1) {
      inner_sr_count += sr->main_signals_.size() - 1;
    }
  }

  auto const ssr_count = inner_sr_count + path_sr_count;

  CHECK_MESSAGE(
      ssr_count <= infra->interlocking_.interlocking_routes_.size(),
      fmt::format("Exptected at least {} signal station routes, but got {}",
                  ssr_count, infra->interlocking_.interlocking_routes_.size()));
}

void check_section_element_types(infrastructure const& infra) {
  for (auto const& section : infra->graph_.sections_) {
    auto const total_track_elements = utls::count_if(
        section.elements_, [](auto&& e) { return e->is_track_element(); });

    CHECK_MESSAGE(total_track_elements == section.elements_.size() - 2,
                  "Only the first and the last element are non-track elements "
                  "(=section elements)");
    CHECK_MESSAGE(!section.elements_.front()->is_track_element(),
                  "Only the first and the last element are non-track elements "
                  "(=section elements)");
    CHECK_MESSAGE(!section.elements_.back()->is_track_element(),
                  "Only the first and the last element are non-track elements "
                  "(=section elements)");
  }
}

void check_section_increasing_kmp(infrastructure const& infra) {
  for (auto const& section : infra->graph_.sections_) {
    for (auto const [e1, e2] : utl::pairwise(section.elements_)) {
      auto const kmp1 = e1->get_km(e2);
      auto const kmp2 = e2->get_km(e1);

      CHECK_MESSAGE(
          kmp1 <= kmp2,
          "Elements in section not in increasing kilometerpoint order.");
    }
  }
}

void check_border_number(infrastructure const& infra) {
  if (infra->stations_.size() == 1) {
    return;  // no border checking with a single station
  }
  auto const total_borders = std::accumulate(
      std::cbegin(infra->stations_), std::cend(infra->stations_),
      std::size_t{0},
      [](auto&& acc, auto&& s) { return acc + s->borders_.size(); });

  CHECK_MESSAGE(total_borders != 0,
                "if there is more than one station we need realized borders");
}

void check_border_pairs(infrastructure const& infra) {

  for (auto const& station_a : infra->stations_) {
    for (auto const& border_a : station_a->borders_) {
      auto const border_b =
          *utls::find_if(border_a.neighbour_->borders_, [&](auto&& border) {
            return border.get_id_tuple() == border_a.get_id_tuple();
          });

      bool const matching_orientation =
          border_a.low_border_ != border_b.low_border_;
      CHECK_MESSAGE(matching_orientation,
                    "Two connected borders should never have opposing mileage");
    }
  }
}

void check_orientation_flags_on_track_elements(infrastructure const& infra) {
  auto const check_node_line = [](node_ptr node) {
    while (node->next_node_ != nullptr &&
           node->next_node_->element_->is_track_element()) {

      bool const same_orientation =
          node->element_->rising() == node->next_node_->element_->rising();
      CHECK_MESSAGE(same_orientation,
                    "Consecutive track elements must have the same orientation "
                    "determined by the rising flag");

      node = node->next_node_;
    }
  };

  for (auto const& element : infra->graph_.elements_) {
    if (element->is_track_element()) {
      continue;
    }

    for (auto const& node : element->nodes()) {
      if (node->next_node_ != nullptr &&
          node->next_node_->element_->is_track_element()) {
        check_node_line(node->next_node_);
      }

      if (node->branch_node_ != nullptr &&
          node->branch_node_->element_->is_track_element()) {
        check_node_line(node->branch_node_);
      }
    }
  }
}

void check_infra(infrastructure const& infra) {
  check_speed_limit_values(infra);
  check_network(infra);
  check_orientation_flags_on_track_elements(infra);
  check_ascending_ids(infra);

  check_station_routes(infra);
  check_station_route_graph(infra);

  check_signal_station_route_count(infra);
  check_signal_station_routes(infra);

  check_section_element_types(infra);
  check_section_increasing_kmp(infra);

  check_border_pairs(infra);
  check_border_number(infra);
}

TEST_SUITE("parse base_infrastructure") {

  TEST_CASE("infra from folder") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    check_infra(infra);
  }

#if defined(SERIALIZE)
  TEST_CASE("serialize infrastructure test") {  // NOLINT
    {
      infrastructure const infra(SMALL_OPTS);
      infra.save("test.raw");
    }

    infrastructure const deserialized("test.raw");
    check_infra(deserialized);
  }
#endif
}

TEST_SUITE("parse infrastructure - de_export" *
           doctest::skip(fs::exists(DE_EXPORT_FOLDER))) {

  TEST_CASE("parse de_export") {
    infrastructure const infra(DE_EXPORT_OPTS);
    check_infra(infra);
  }

}
