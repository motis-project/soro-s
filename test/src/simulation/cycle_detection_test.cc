#include "doctest/doctest.h"

#include "soro/simulation/sim_graph.h"

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::simulation;

// turn off warning for this suite
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

TEST_SUITE("cycle_detection suite") {
  TEST_CASE("no cycle") {
    infrastructure const infra;
    timetable const tt;

    // graph: 0 -> 1 -> 2 -> 3 -> 4
    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1}});
    sg.nodes_.push_back(sim_node{.out_ = {2}});
    sg.nodes_.push_back(sim_node{.out_ = {3}});
    sg.nodes_.push_back(sim_node{.out_ = {4}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(0, 4);

    auto const result = sg.has_cycle();

    CHECK(!result);
  }

  TEST_CASE("cycle") {
    infrastructure const infra;
    timetable const tt;
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1}});
    sg.nodes_.push_back(sim_node{.out_ = {2}});
    sg.nodes_.push_back(sim_node{.out_ = {3}});
    sg.nodes_.push_back(sim_node{.out_ = {4}});
    sg.nodes_.push_back(sim_node{.out_ = {0}});
    sg.train_to_sim_nodes_.emplace_back(0, 4);

    auto const result = sg.has_cycle();

    CHECK(result);
  }

  TEST_CASE("no cycle in diamond") {
    infrastructure const infra;
    timetable const tt;
    /* graph:
          0
         / \
        1   2
        \  /
         3      */

    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1, 2}});
    sg.nodes_.push_back(sim_node{.out_ = {3}});
    sg.nodes_.push_back(sim_node{.out_ = {3}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(0, 3);

    auto const result = sg.has_cycle();

    CHECK(!result);
  }

  TEST_CASE("no cycle forest") {
    infrastructure const infra;
    timetable const tt;
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1}});
    sg.nodes_.push_back(sim_node{.out_ = {2}});
    sg.nodes_.push_back(sim_node{.out_ = {3}});
    sg.nodes_.push_back(sim_node{.out_ = {4}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(0, 4);

    sg.nodes_.push_back(sim_node{.out_ = {6}});
    sg.nodes_.push_back(sim_node{.out_ = {7}});
    sg.nodes_.push_back(sim_node{.out_ = {8}});
    sg.nodes_.push_back(sim_node{.out_ = {9}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(5, 9);

    auto const result = sg.has_cycle();

    CHECK(!result);
  }

  TEST_CASE("cycle forest") {
    infrastructure const infra;
    timetable const tt;
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9 -> 5
    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1}});
    sg.nodes_.push_back(sim_node{.out_ = {2}});
    sg.nodes_.push_back(sim_node{.out_ = {3}});
    sg.nodes_.push_back(sim_node{.out_ = {4}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(0, 4);

    sg.nodes_.push_back(sim_node{.out_ = {6}});
    sg.nodes_.push_back(sim_node{.out_ = {7}});
    sg.nodes_.push_back(sim_node{.out_ = {8}});
    sg.nodes_.push_back(sim_node{.out_ = {9}});
    sg.nodes_.push_back(sim_node{.out_ = {5}});
    sg.train_to_sim_nodes_.emplace_back(5, 9);

    auto const result = sg.has_cycle();

    CHECK(result);
  }

  TEST_CASE("cycle forest 2") {
    infrastructure const infra;
    timetable const tt;
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1}});
    sg.nodes_.push_back(sim_node{.out_ = {2}});
    sg.nodes_.push_back(sim_node{.out_ = {3}});
    sg.nodes_.push_back(sim_node{.out_ = {4}});
    sg.nodes_.push_back(sim_node{.out_ = {0}});
    sg.train_to_sim_nodes_.emplace_back(0, 4);

    sg.nodes_.push_back(sim_node{.out_ = {6}});
    sg.nodes_.push_back(sim_node{.out_ = {7}});
    sg.nodes_.push_back(sim_node{.out_ = {8}});
    sg.nodes_.push_back(sim_node{.out_ = {9}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(5, 9);

    auto const result = sg.has_cycle();

    CHECK(result);
  }

  TEST_CASE("cycle from following") {
    infrastructure const infra;
    timetable const tt;
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        15 -> 5, 16 -> 6, 17 -> 7
    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1}});
    sg.nodes_.push_back(sim_node{.out_ = {2}});
    sg.nodes_.push_back(sim_node{.out_ = {3, 10}});
    sg.nodes_.push_back(sim_node{.out_ = {4, 11}});
    sg.nodes_.push_back(sim_node{.out_ = {5, 12}});
    sg.nodes_.push_back(sim_node{.out_ = {6, 13}});
    sg.nodes_.push_back(sim_node{.out_ = {7}});
    sg.nodes_.push_back(sim_node{.out_ = {8}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(0, 8);

    sg.nodes_.push_back(sim_node{.out_ = {10}});
    sg.nodes_.push_back(sim_node{.out_ = {11}});
    sg.nodes_.push_back(sim_node{.out_ = {12}});
    sg.nodes_.push_back(sim_node{.out_ = {13}});
    sg.nodes_.push_back(sim_node{.out_ = {14}});
    sg.nodes_.push_back(sim_node{.out_ = {15}});
    sg.nodes_.push_back(sim_node{.out_ = {16, 5}});
    sg.nodes_.push_back(sim_node{.out_ = {17, 6}});
    sg.nodes_.push_back(sim_node{.out_ = {7}});
    sg.train_to_sim_nodes_.emplace_back(9, 17);

    auto const result = sg.has_cycle();

    CHECK(result);
  }

  TEST_CASE("no cycle from following") {
    infrastructure const infra;
    timetable const tt;
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        16 -> 6, 17 -> 7
    sim_graph sg(infra, tt);

    sg.nodes_.push_back(sim_node{.out_ = {1}});
    sg.nodes_.push_back(sim_node{.out_ = {2}});
    sg.nodes_.push_back(sim_node{.out_ = {3, 10}});
    sg.nodes_.push_back(sim_node{.out_ = {4, 11}});
    sg.nodes_.push_back(sim_node{.out_ = {5, 12}});
    sg.nodes_.push_back(sim_node{.out_ = {6, 13}});
    sg.nodes_.push_back(sim_node{.out_ = {7}});
    sg.nodes_.push_back(sim_node{.out_ = {8}});
    sg.nodes_.push_back(sim_node{.out_ = {}});
    sg.train_to_sim_nodes_.emplace_back(0, 8);

    sg.nodes_.push_back(sim_node{.out_ = {10}});
    sg.nodes_.push_back(sim_node{.out_ = {11}});
    sg.nodes_.push_back(sim_node{.out_ = {12}});
    sg.nodes_.push_back(sim_node{.out_ = {13}});
    sg.nodes_.push_back(sim_node{.out_ = {14}});
    sg.nodes_.push_back(sim_node{.out_ = {15}});
    sg.nodes_.push_back(sim_node{.out_ = {16}});
    sg.nodes_.push_back(sim_node{.out_ = {17, 6}});
    sg.nodes_.push_back(sim_node{.out_ = {7}});
    sg.train_to_sim_nodes_.emplace_back(9, 17);

    auto const result = sg.has_cycle();

    CHECK(!result);
  }
}

#pragma GCC diagnostic pop
