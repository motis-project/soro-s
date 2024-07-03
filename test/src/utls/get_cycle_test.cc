#include "doctest/doctest.h"

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "utl/pairwise.h"

#include "soro/base/soro_types.h"

#include "soro/utls/std_wrapper/contains.h"

#include "test/utls/get_cycle.h"

namespace soro::test {

struct graph {
  struct node {
    node(uint32_t id, std::initializer_list<uint32_t> out) : id_{id} {
      for (auto const o : out) {
        out_.emplace_back(o);
      }
    }

    using id = soro::strong<uint32_t, struct _test_node_id>;

    id get_id(graph const&) const { return id_; }

    id id_;
    std::vector<id> out_;
  };

  std::vector<node::id> out(node::id const& id) const {
    return nodes_[id].out_;
  }

  soro::vector_map<node::id, node> nodes_;
};

TEST_SUITE("cycle_detection suite") {

  void check_is_valid_cycle(cycle<graph::node::id> const& c, graph const& og) {
    for (auto const [from, to] : utl::pairwise(c)) {
      CHECK(utls::contains(og.nodes_[from].out_, to));
    }

    CHECK(utls::contains(og.nodes_[c.back()].out_, c.front()));
  }

  TEST_CASE("no cycle") {
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3}});
    g.nodes_.emplace_back(graph::node{3, {4}});
    g.nodes_.emplace_back(graph::node{4, {}});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3}});
    g.nodes_.emplace_back(graph::node{3, {4}});
    g.nodes_.emplace_back(graph::node{4, {0}});

    auto result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("no cycle in diamond") {
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1, 2}});
    g.nodes_.emplace_back(graph::node{1, {3}});
    g.nodes_.emplace_back(graph::node{2, {3}});
    g.nodes_.emplace_back(graph::node{3, {}});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("no cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3}});
    g.nodes_.emplace_back(graph::node{3, {4}});
    g.nodes_.emplace_back(graph::node{4, {}});

    g.nodes_.emplace_back(graph::node{5, {6}});
    g.nodes_.emplace_back(graph::node{6, {7}});
    g.nodes_.emplace_back(graph::node{7, {8}});
    g.nodes_.emplace_back(graph::node{8, {9}});
    g.nodes_.emplace_back(graph::node{9, {}});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("cycle forest") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    // graph: 5 -> 6 -> 7 -> 8 -> 9 -> 5
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3}});
    g.nodes_.emplace_back(graph::node{3, {4}});
    g.nodes_.emplace_back(graph::node{4, {}});

    g.nodes_.emplace_back(graph::node{5, {6}});
    g.nodes_.emplace_back(graph::node{6, {7}});
    g.nodes_.emplace_back(graph::node{7, {8}});
    g.nodes_.emplace_back(graph::node{8, {9}});
    g.nodes_.emplace_back(graph::node{9, {5}});

    auto const result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("cycle forest 2") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    // graph: 5 -> 6 -> 7 -> 8 -> 9
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3}});
    g.nodes_.emplace_back(graph::node{3, {4}});
    g.nodes_.emplace_back(graph::node{4, {0}});

    g.nodes_.emplace_back(graph::node{5, {6}});
    g.nodes_.emplace_back(graph::node{6, {7}});
    g.nodes_.emplace_back(graph::node{7, {8}});
    g.nodes_.emplace_back(graph::node{8, {9}});
    g.nodes_.emplace_back(graph::node{9, {}});

    auto const result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        15 -> 5, 16 -> 6, 17 -> 7
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3, 10}});
    g.nodes_.emplace_back(graph::node{3, {4, 11}});
    g.nodes_.emplace_back(graph::node{4, {5, 12}});
    g.nodes_.emplace_back(graph::node{5, {6, 13}});
    g.nodes_.emplace_back(graph::node{6, {7}});
    g.nodes_.emplace_back(graph::node{7, {8}});
    g.nodes_.emplace_back(graph::node{8, {}});

    g.nodes_.emplace_back(graph::node{9, {10}});
    g.nodes_.emplace_back(graph::node{10, {11}});
    g.nodes_.emplace_back(graph::node{11, {12}});
    g.nodes_.emplace_back(graph::node{12, {13}});
    g.nodes_.emplace_back(graph::node{13, {14}});
    g.nodes_.emplace_back(graph::node{14, {15}});
    g.nodes_.emplace_back(graph::node{15, {16, 5}});
    g.nodes_.emplace_back(graph::node{16, {17, 6}});
    g.nodes_.emplace_back(graph::node{17, {7}});

    auto const result = get_cycle(g);

    CHECK(result);
    check_is_valid_cycle(*result, g);  // NOLINT
  }

  TEST_CASE("no cycle from following") {
    // graph: 0 ->  1 ->  2 ->  3 ->  4 ->  5 ->  6 ->  7 ->  8
    //              9 -> 10 -> 11 -> 12 -> 13 -> 14 -> 15 -> 16 -> 17
    //        2 -> 10, 3 -> 11, 4 -> 12, 5 -> 13
    //        16 -> 6, 17 -> 7
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3, 10}});
    g.nodes_.emplace_back(graph::node{3, {4, 11}});
    g.nodes_.emplace_back(graph::node{4, {5, 12}});
    g.nodes_.emplace_back(graph::node{5, {6, 13}});
    g.nodes_.emplace_back(graph::node{6, {7}});
    g.nodes_.emplace_back(graph::node{7, {8}});
    g.nodes_.emplace_back(graph::node{8, {}});

    g.nodes_.emplace_back(graph::node{9, {10}});
    g.nodes_.emplace_back(graph::node{10, {11}});
    g.nodes_.emplace_back(graph::node{11, {12}});
    g.nodes_.emplace_back(graph::node{12, {13}});
    g.nodes_.emplace_back(graph::node{13, {14}});
    g.nodes_.emplace_back(graph::node{14, {15}});
    g.nodes_.emplace_back(graph::node{15, {16}});
    g.nodes_.emplace_back(graph::node{16, {17, 6}});
    g.nodes_.emplace_back(graph::node{17, {7}});

    auto const result = get_cycle(g);

    CHECK(!result);
  }

  TEST_CASE("two cycles") {
    // graph: 0 -> 1 -> 2 -> 3 -> 4
    //        3 -> 1
    //        4 -> 0
    graph g;

    g.nodes_.emplace_back(graph::node{0, {1}});
    g.nodes_.emplace_back(graph::node{1, {2}});
    g.nodes_.emplace_back(graph::node{2, {3}});
    g.nodes_.emplace_back(graph::node{3, {1, 4}});

    g.nodes_.emplace_back(graph::node{4, {0}});

    auto const result = get_cycle(g);

    CHECK(result);
    CHECK_EQ(result->size(), 3);  // NOLINT
    check_is_valid_cycle(*result, g);  // NOLINT
  }
}

}  // namespace soro::test
