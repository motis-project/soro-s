#include "doctest/doctest.h"

#include "utl/pairwise.h"

#include "soro/utls/std_wrapper/contains.h"

#include "soro/simulation/ordering/get_cycle.h"

namespace soro::simulation {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

TEST_SUITE("cycle_detection suite") {

  void check_is_valid_cycle(cycle const& c, ordering_graph const& og) {
    for (auto const [from, to] : utl::pairwise(c)) {
      CHECK(utls::contains(og.nodes_[from].out_, to));
    }

    CHECK(utls::contains(og.nodes_[c.back()].out_, c.front()));
  }

  TEST_CASE("no cycle") {
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3}});
    g.nodes_.push_back({.id_ = 3, .out_ = {4}});
    g.nodes_.push_back({.id_ = 4});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3}});
    g.nodes_.push_back({.id_ = 3, .out_ = {4}});
    g.nodes_.push_back({.id_ = 4, .out_ = {0}});

    auto result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("no cycle in diamond") {
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1, 2}});
    g.nodes_.push_back({.id_ = 1, .out_ = {3}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3}});
    g.nodes_.push_back({.id_ = 3});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("no cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3}});
    g.nodes_.push_back({.id_ = 3, .out_ = {4}});
    g.nodes_.push_back({.id_ = 4});

    g.nodes_.push_back({.id_ = 5, .out_ = {6}});
    g.nodes_.push_back({.id_ = 6, .out_ = {7}});
    g.nodes_.push_back({.id_ = 7, .out_ = {8}});
    g.nodes_.push_back({.id_ = 8, .out_ = {9}});
    g.nodes_.push_back({.id_ = 9});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9 -> 5
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3}});
    g.nodes_.push_back({.id_ = 3, .out_ = {4}});
    g.nodes_.push_back({.id_ = 4, .out_ = {}});

    g.nodes_.push_back({.id_ = 5, .out_ = {6}});
    g.nodes_.push_back({.id_ = 6, .out_ = {7}});
    g.nodes_.push_back({.id_ = 7, .out_ = {8}});
    g.nodes_.push_back({.id_ = 8, .out_ = {9}});
    g.nodes_.push_back({.id_ = 9, .out_ = {5}});

    auto const result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("cycle forest 2") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3}});
    g.nodes_.push_back({.id_ = 3, .out_ = {4}});
    g.nodes_.push_back({.id_ = 4, .out_ = {0}});

    g.nodes_.push_back({.id_ = 5, .out_ = {6}});
    g.nodes_.push_back({.id_ = 6, .out_ = {7}});
    g.nodes_.push_back({.id_ = 7, .out_ = {8}});
    g.nodes_.push_back({.id_ = 8, .out_ = {9}});
    g.nodes_.push_back({.id_ = 9, .out_ = {}});

    auto const result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        15 -> 5, 16 -> 6, 17 -> 7
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3, 10}});
    g.nodes_.push_back({.id_ = 3, .out_ = {4, 11}});
    g.nodes_.push_back({.id_ = 4, .out_ = {5, 12}});
    g.nodes_.push_back({.id_ = 5, .out_ = {6, 13}});
    g.nodes_.push_back({.id_ = 6, .out_ = {7}});
    g.nodes_.push_back({.id_ = 7, .out_ = {8}});
    g.nodes_.push_back({.id_ = 8, .out_ = {}});

    g.nodes_.push_back({.id_ = 9, .out_ = {10}});
    g.nodes_.push_back({.id_ = 10, .out_ = {11}});
    g.nodes_.push_back({.id_ = 11, .out_ = {12}});
    g.nodes_.push_back({.id_ = 12, .out_ = {13}});
    g.nodes_.push_back({.id_ = 13, .out_ = {14}});
    g.nodes_.push_back({.id_ = 14, .out_ = {15}});
    g.nodes_.push_back({.id_ = 15, .out_ = {16, 5}});
    g.nodes_.push_back({.id_ = 16, .out_ = {17, 6}});
    g.nodes_.push_back({.id_ = 17, .out_ = {7}});

    auto const result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("no cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        16 -> 6, 17 -> 7
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3, 10}});
    g.nodes_.push_back({.id_ = 3, .out_ = {4, 11}});
    g.nodes_.push_back({.id_ = 4, .out_ = {5, 12}});
    g.nodes_.push_back({.id_ = 5, .out_ = {6, 13}});
    g.nodes_.push_back({.id_ = 6, .out_ = {7}});
    g.nodes_.push_back({.id_ = 7, .out_ = {8}});
    g.nodes_.push_back({.id_ = 8, .out_ = {}});

    g.nodes_.push_back({.id_ = 9, .out_ = {10}});
    g.nodes_.push_back({.id_ = 10, .out_ = {11}});
    g.nodes_.push_back({.id_ = 11, .out_ = {12}});
    g.nodes_.push_back({.id_ = 12, .out_ = {13}});
    g.nodes_.push_back({.id_ = 13, .out_ = {14}});
    g.nodes_.push_back({.id_ = 14, .out_ = {15}});
    g.nodes_.push_back({.id_ = 15, .out_ = {16}});
    g.nodes_.push_back({.id_ = 16, .out_ = {17, 6}});
    g.nodes_.push_back({.id_ = 17, .out_ = {7}});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("two cycles") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    //        3 -> 1
    //        4 -> 0
    ordering_graph g;

    g.nodes_.push_back({.id_ = 0, .out_ = {1}});
    g.nodes_.push_back({.id_ = 1, .out_ = {2}});
    g.nodes_.push_back({.id_ = 2, .out_ = {3}});
    g.nodes_.push_back({.id_ = 3, .out_ = {1, 4}});

    g.nodes_.push_back({.id_ = 4, .out_ = {0}});

    auto const result = get_cycle(g);

    CHECK(result);
    CHECK_EQ(result->size(), 3);  // NOLINT
    check_is_valid_cycle(*result, g);  // NOLINT
  }
}

#pragma GCC diagnostic pop

}  // namespace soro::simulation
