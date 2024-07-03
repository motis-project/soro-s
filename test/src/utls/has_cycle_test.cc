#include "doctest/doctest.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "test/utls/has_cycle.h"

TEST_SUITE("cycle_detection suite") {
  struct graph {
    struct node {
      using id = uint32_t;
      node(id id, std::vector<uint32_t> out) : id_{id}, out_{std::move(out)} {}

      id get_id(graph const&) const { return id_; }

      id id_;
      std::vector<uint32_t> out_;
    };

    std::vector<node::id> out(node::id const& id) const {
      return nodes_[id].out_;
    }

    std::vector<node> nodes_;
  };

  bool detect_cycle(graph const& graph) { return soro::test::has_cycle(graph); }

  TEST_CASE("no cycle") {

    graph g;

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{2});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{4});
    g.nodes_.emplace_back(4, std::vector<uint32_t>{});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    graph g;

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{2});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{4});
    g.nodes_.emplace_back(4, std::vector<uint32_t>{0});

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

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1, 2});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{3});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("no cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    graph g;

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{2});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{4});
    g.nodes_.emplace_back(4, std::vector<uint32_t>{});

    g.nodes_.emplace_back(5, std::vector<uint32_t>{6});
    g.nodes_.emplace_back(6, std::vector<uint32_t>{7});
    g.nodes_.emplace_back(7, std::vector<uint32_t>{8});
    g.nodes_.emplace_back(8, std::vector<uint32_t>{9});
    g.nodes_.emplace_back(9, std::vector<uint32_t>{});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9 -> 5
    graph g;

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{2});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{4});
    g.nodes_.emplace_back(4, std::vector<uint32_t>{});

    g.nodes_.emplace_back(5, std::vector<uint32_t>{6});
    g.nodes_.emplace_back(6, std::vector<uint32_t>{7});
    g.nodes_.emplace_back(7, std::vector<uint32_t>{8});
    g.nodes_.emplace_back(8, std::vector<uint32_t>{9});
    g.nodes_.emplace_back(9, std::vector<uint32_t>{5});

    auto const result = detect_cycle(g);

    CHECK(result);
  }

  TEST_CASE("cycle forest 2") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    graph g;

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{2});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{4});
    g.nodes_.emplace_back(4, std::vector<uint32_t>{0});

    g.nodes_.emplace_back(5, std::vector<uint32_t>{6});
    g.nodes_.emplace_back(6, std::vector<uint32_t>{7});
    g.nodes_.emplace_back(7, std::vector<uint32_t>{8});
    g.nodes_.emplace_back(8, std::vector<uint32_t>{9});
    g.nodes_.emplace_back(9, std::vector<uint32_t>{});

    auto const result = detect_cycle(g);

    CHECK(result);
  }

  TEST_CASE("cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        15 -> 5, 16 -> 6, 17 -> 7
    graph g;

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{2});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3, 10});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{4, 11});
    g.nodes_.emplace_back(4, std::vector<uint32_t>{5, 12});
    g.nodes_.emplace_back(5, std::vector<uint32_t>{6, 13});
    g.nodes_.emplace_back(6, std::vector<uint32_t>{7});
    g.nodes_.emplace_back(7, std::vector<uint32_t>{8});
    g.nodes_.emplace_back(8, std::vector<uint32_t>{});

    g.nodes_.emplace_back(9, std::vector<uint32_t>{10});
    g.nodes_.emplace_back(10, std::vector<uint32_t>{11});
    g.nodes_.emplace_back(11, std::vector<uint32_t>{12});
    g.nodes_.emplace_back(12, std::vector<uint32_t>{13});
    g.nodes_.emplace_back(13, std::vector<uint32_t>{14});
    g.nodes_.emplace_back(14, std::vector<uint32_t>{15});
    g.nodes_.emplace_back(15, std::vector<uint32_t>{16, 5});
    g.nodes_.emplace_back(16, std::vector<uint32_t>{17, 6});
    g.nodes_.emplace_back(17, std::vector<uint32_t>{7});

    auto const result = detect_cycle(g);

    CHECK(result);
  }

  TEST_CASE("no cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        16 -> 6, 17 -> 7
    graph g;

    g.nodes_.emplace_back(0, std::vector<uint32_t>{1});
    g.nodes_.emplace_back(1, std::vector<uint32_t>{2});
    g.nodes_.emplace_back(2, std::vector<uint32_t>{3, 10});
    g.nodes_.emplace_back(3, std::vector<uint32_t>{4, 11});
    g.nodes_.emplace_back(4, std::vector<uint32_t>{5, 12});
    g.nodes_.emplace_back(5, std::vector<uint32_t>{6, 13});
    g.nodes_.emplace_back(6, std::vector<uint32_t>{7});
    g.nodes_.emplace_back(7, std::vector<uint32_t>{8});
    g.nodes_.emplace_back(8, std::vector<uint32_t>{});

    g.nodes_.emplace_back(9, std::vector<uint32_t>{10});
    g.nodes_.emplace_back(10, std::vector<uint32_t>{11});
    g.nodes_.emplace_back(11, std::vector<uint32_t>{12});
    g.nodes_.emplace_back(12, std::vector<uint32_t>{13});
    g.nodes_.emplace_back(13, std::vector<uint32_t>{14});
    g.nodes_.emplace_back(14, std::vector<uint32_t>{15});
    g.nodes_.emplace_back(15, std::vector<uint32_t>{16});
    g.nodes_.emplace_back(16, std::vector<uint32_t>{17, 6});
    g.nodes_.emplace_back(17, std::vector<uint32_t>{7});

    auto const result = detect_cycle(g);

    CHECK(!result);
  }
}