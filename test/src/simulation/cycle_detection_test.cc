#include "doctest/doctest.h"

#include "soro/utls/graph/has_cycle.h"

TEST_SUITE("cycle_detection suite") {
  using graph = std::vector<std::vector<uint32_t>>;

  bool detect_cycle(graph const& graph) {
    return soro::utls::has_cycle(graph,
                                 [](auto&& g, auto&& id) { return g[id]; });
  }

  TEST_CASE("no cycle") {

    graph g;

    g.push_back({1});
    g.push_back({2});
    g.push_back({3});
    g.push_back({4});
    g.push_back({});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    graph g;

    g.push_back({1});
    g.push_back({2});
    g.push_back({3});
    g.push_back({4});
    g.push_back({0});

    auto const result = detect_cycle(g);

    CHECK(result);
  }

  TEST_CASE("no cycle in diamond") {
    /* graph:
          0
         / \
        1   2
        \  /
         3      */

    graph g;

    g.push_back({1, 2});
    g.push_back({3});
    g.push_back({3});
    g.push_back({});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("no cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    graph g;

    g.push_back({1});
    g.push_back({2});
    g.push_back({3});
    g.push_back({4});
    g.push_back({});

    g.push_back({6});
    g.push_back({7});
    g.push_back({8});
    g.push_back({9});
    g.push_back({});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9 -> 5
    graph g;

    g.push_back({1});
    g.push_back({2});
    g.push_back({3});
    g.push_back({4});
    g.push_back({});

    g.push_back({6});
    g.push_back({7});
    g.push_back({8});
    g.push_back({9});
    g.push_back({5});

    auto const result = detect_cycle(g);

    CHECK(result);
  }

  TEST_CASE("cycle forest 2") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    graph g;

    g.push_back({1});
    g.push_back({2});
    g.push_back({3});
    g.push_back({4});
    g.push_back({0});

    g.push_back({6});
    g.push_back({7});
    g.push_back({8});
    g.push_back({9});
    g.push_back({});

    auto const result = detect_cycle(g);

    CHECK(result);
  }

  TEST_CASE("cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        15 -> 5, 16 -> 6, 17 -> 7
    graph g;

    g.push_back({1});
    g.push_back({2});
    g.push_back({3, 10});
    g.push_back({4, 11});
    g.push_back({5, 12});
    g.push_back({6, 13});
    g.push_back({7});
    g.push_back({8});
    g.push_back({});

    g.push_back({10});
    g.push_back({11});
    g.push_back({12});
    g.push_back({13});
    g.push_back({14});
    g.push_back({15});
    g.push_back({16, 5});
    g.push_back({17, 6});
    g.push_back({7});

    auto const result = detect_cycle(g);

    CHECK(result);
  }

  TEST_CASE("no cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        16 -> 6, 17 -> 7
    graph g;

    g.push_back({1});
    g.push_back({2});
    g.push_back({3, 10});
    g.push_back({4, 11});
    g.push_back({5, 12});
    g.push_back({6, 13});
    g.push_back({7});
    g.push_back({8});
    g.push_back({});

    g.push_back({10});
    g.push_back({11});
    g.push_back({12});
    g.push_back({13});
    g.push_back({14});
    g.push_back({15});
    g.push_back({16});
    g.push_back({17, 6});
    g.push_back({7});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }
}