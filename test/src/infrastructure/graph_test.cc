#include "test/infrastructure/graph_test.h"

#include "doctest/doctest.h"
#include "fmt/format.h"

#include "soro/utls/container/constexpr_map.h"

namespace soro::infra::test {

constexpr std::array<std::pair<type, size_t>,
                     static_cast<std::size_t>(type::INVALID) + 1> const
    expected_edges_arr{{// end elements
                        {type::BUMPER, 1},
                        {type::TRACK_END, 1},
                        // simple elements
                        {type::KM_JUMP, 2},
                        {type::BORDER, 1},
                        {type::LINE_SWITCH, 2},
                        // simple switch
                        {type::SIMPLE_SWITCH, 3},
                        // cross
                        {type::CROSS, 4},
                        // directed track elements
                        {type::MAIN_SIGNAL, 1},
                        {type::PROTECTION_SIGNAL, 1},
                        {type::APPROACH_SIGNAL, 1},
                        {type::RUNTIME_CHECKPOINT, 1},
                        {type::EOTD, 1},
                        {type::SPEED_LIMIT, 1},
                        {type::POINT_SPEED, 1},
                        {type::BRAKE_PATH, 1},
                        {type::LZB_START, 1},
                        {type::LZB_END, 1},
                        {type::LZB_BLOCK_SIGN, 1},
                        {type::ETCS_START, 1},
                        {type::ETCS_END, 1},
                        {type::ETCS_BLOCK_SIGN, 1},
                        {type::FORCED_HALT, 1},
                        {type::HALT, 1},
                        // undirected track elements
                        {type::TUNNEL, 2},
                        {type::ENTRY, 2},
                        {type::TRACK_NAME, 2},
                        {type::RUNTIME_CHECKPOINT_UNDIRECTED, 2},
                        {type::LEVEL_CROSSING, 2},
                        {type::SLOPE, 2},
                        //
                        {type::INVALID, 0}}};

// Please add the missing types to the expected edges array
static_assert(expected_edges_arr.back().first == type::INVALID &&
              expected_edges_arr.back().second == 0);

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

    CHECK_MESSAGE((stem_id == (ss.stem_rising_ ? ss.falling_stem_neighbour()
                                               : ss.rising_stem_neighbour())
                                  ->id()),
                  in_station);

    auto const branch_id = (take_branch ? node->branch_node_ : node->next_node_)
                               ->branch_node_->element_->id();
    CHECK_MESSAGE(
        (branch_id == (ss.branch_rising_ ? ss.falling_branch_neighbour()
                                         : ss.rising_branch_neighbour())
                          ->id()),
        in_station);
  }

  {  // check stem -> start
    auto const& [node, take_branch] =
        get_node((ss.stem_rising_ ? *ss.rising_stem_neighbour()
                                  : *ss.falling_stem_neighbour()),
                 ss.id_);

    auto const start_id = (take_branch ? node->branch_node_ : node->next_node_)
                              ->next_node_->element_->id();

    CHECK_MESSAGE((start_id == (ss.rising_ ? ss.falling_start_neighbour()
                                           : ss.rising_start_neighbour())
                                   ->id()),
                  in_station);
  }

  {  // check branch -> start
    auto const& [node, take_branch] =
        get_node((ss.branch_rising_ ? *ss.rising_branch_neighbour()
                                    : *ss.falling_branch_neighbour()),
                 ss.id_);

    auto const start_id = (take_branch ? node->branch_node_ : node->next_node_)
                              ->next_node_->element_->id();

    CHECK_MESSAGE((start_id == (ss.rising_ ? ss.falling_start_neighbour()
                                           : ss.rising_start_neighbour())
                                   ->id()),
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
        (end_left_id == (cross.end_left_rising_ ? cross.falling_end_left()
                                                : cross.rising_end_left())
                            ->id()),
        in_station);

    if (cross.start_left_end_right_arc_) {
      auto const end_right_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE(
          (end_right_id == (cross.end_right_rising_ ? cross.falling_end_right()
                                                    : cross.rising_end_right())
                               ->id()),
          in_station);
    }
  }

  {  // check end_left -> start_left and end_left -> start_right
    auto const cross_node =
        get_cross_node((cross.end_left_rising_ ? *cross.rising_end_left()
                                               : *cross.falling_end_left()),
                       cross.id_);

    auto const start_left_id = cross_node->next_node_->element_->id();
    CHECK_MESSAGE((start_left_id == (cross.rising_ ? cross.falling_start_left()
                                                   : cross.rising_start_left())
                                        ->id()),
                  in_station);

    if (cross.start_right_end_left_arc_) {
      auto const start_right_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE((start_right_id == (cross.start_right_rising_
                                            ? cross.falling_start_right()
                                            : cross.rising_start_right())
                                           ->id()),
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
        (end_right_id == (cross.end_right_rising_ ? cross.falling_end_right()
                                                  : cross.rising_end_right())
                             ->id()),
        in_station);

    if (cross.start_right_end_left_arc_) {
      auto const end_left_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE(
          (end_left_id == (cross.end_left_rising_ ? cross.falling_end_left()
                                                  : cross.rising_end_left())
                              ->id()),
          in_station);
    }
  }

  {  // check end_right -> start_right and end_right -> start_left
    auto const cross_node =
        get_cross_node((cross.end_right_rising_ ? *cross.rising_end_right()
                                                : *cross.falling_end_right()),
                       cross.id_);

    auto const start_right_id = cross_node->next_node_->element_->id();
    CHECK_MESSAGE((start_right_id == (cross.start_right_rising_
                                          ? cross.falling_start_right()
                                          : cross.rising_start_right())
                                         ->id()),
                  in_station);

    if (cross.start_left_end_right_arc_) {
      auto const start_left_id = cross_node->branch_node_->element_->id();
      CHECK_MESSAGE(
          (start_left_id == (cross.rising_ ? cross.falling_start_left()
                                           : cross.rising_start_left())
                                ->id()),
          in_station);
    }
  }
}

void check_outgoing(element const& element, station const& station) {
  std::size_t outgoing = 0;
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

  CHECK_MESSAGE((expected == outgoing),
                fmt::format("Expected outgoing edges differ from actual "
                            "outgoing edges for element with id: {}.",
                            element.id()));
}

void check_incoming(infrastructure const& infra,
                    std::vector<std::size_t> const& incoming_edges,
                    std::vector<std::size_t> const& expected_incoming) {
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
        CHECK_MESSAGE((expected == incoming), message);
      }
    }
  }
}

void check_neighbours(element const& e) {
  for (auto const& neigh : e.neighbours()) {
    CHECK_NE(neigh, nullptr);
  }
}

void check_graph(infrastructure const& infra) {
  std::size_t total_elements = 0;
  std::vector<std::size_t> incoming_edges(infra->graph_.elements_.size(), 0);
  std::vector<std::size_t> expected_incoming(infra->graph_.elements_.size(), 0);

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

          CHECK_MESSAGE((node_ptr->next_node_->next_node_ != node_ptr),
                        cycle_message);
          CHECK_MESSAGE((node_ptr->next_node_->branch_node_ != node_ptr),
                        cycle_message);
        }

        if (node_ptr->branch_node_ != nullptr) {
          ++(incoming_edges[node_ptr->branch_node_->element_->id()]);

          CHECK_MESSAGE((node_ptr->branch_node_->next_node_ != node_ptr),
                        cycle_message);
          CHECK_MESSAGE((node_ptr->branch_node_->branch_node_ != node_ptr),
                        cycle_message);
        }
      }
    }
  }

  check_incoming(infra, incoming_edges, expected_incoming);

  CHECK_EQ(total_elements, infra->graph_.elements_.size());
}

void do_graph_tests(infrastructure const& infra) { check_graph(infra); }

}  // namespace soro::infra::test
