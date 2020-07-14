#include <iostream>
#include <string>

#include "doctest/doctest.h"

#include "soro/ascii_network_parser.h"

using namespace soro;
namespace cr = cista::raw;

TEST_CASE("simple_track_horizontal") {
  auto const level_junction = R"(
# 1 Simple Horizontal Track with Name

==1337=

)";
  auto const net = parse_network(level_junction);
  CHECK(net.nodes_.empty());
  CHECK(net.edges_.size() == 1U);
  CHECK(net.edges_[0]->dist_ == 7U);
  CHECK(net.edges_[0]->id_ == 1337U);
}

TEST_CASE("simple_track_vertical") {
  auto const level_junction = R"(
# 1 Simple Vertical Track with Name

    |
    |
    1
    2
    |

)";
  auto const net = parse_network(level_junction);
  CHECK(net.nodes_.empty());
  CHECK(net.edges_.size() == 1U);
  CHECK(net.edges_[0]->dist_ == 5U);
  CHECK(net.edges_[0]->id_ == 12U);
}

TEST_CASE("simple_track_diagonal_left_to_right") {
  auto const level_junction = R"(
# 1 Simple Diagonal Track Left to Right with Name

  \
   \
    3
     4
      \
       \

)";
  auto const net = parse_network(level_junction);
  CHECK(net.nodes_.empty());
  CHECK(net.edges_.size() == 1U);
  CHECK(net.edges_[0]->dist_ == 6U);
  CHECK(net.edges_[0]->id_ == 34U);
}

TEST_CASE("simple_track_diagonal_right_to_left") {
  auto const level_junction = R"(
# 1 Simple Diagonal Track Right to Left with Name

      /
     /
    3
   4
  /
 /

)";
  auto const net = parse_network(level_junction);
  CHECK(net.nodes_.empty());
  CHECK(net.edges_.size() == 1U);
  CHECK(net.edges_[0]->dist_ == 6U);
  CHECK(net.edges_[0]->id_ == 34U);
}

TEST_CASE("upper_l") {
  auto const s = R"(
# 1 Upper L

 *=9=
 |
 |

)";
  auto const net = parse_network(s);
  CHECK(net.nodes_.empty());
  CHECK(net.edges_.size() == 1U);
  CHECK(net.edges_[0]->dist_ == 6U);
  CHECK(net.edges_[0]->id_ == 9U);
}

TEST_CASE("lower_l") {
  auto const s = R"(
# 1 Lower L

    |
    |
 =3=*

)";
  auto const net = parse_network(s);
  CHECK(net.nodes_.empty());
  CHECK(net.edges_.size() == 1U);
  CHECK(net.edges_[0]->dist_ == 6U);
  CHECK(net.edges_[0]->id_ == 3U);
}

TEST_CASE("simple_track_rectangle") {
  auto const s = R"(
# 1 Rectangle

 *=7=*
 |   |
 |   |
 *===*

)";
  auto const net = parse_network(s);
  REQUIRE(net.nodes_.size() == 1U);
  REQUIRE(net.edges_.size() == 1U);
  CHECK(net.edges_[0]->dist_ == 14U);
  CHECK(net.edges_[0]->id_ == 7U);
  CHECK(net.edges_[0]->from_ == net.nodes_[0].get());
  CHECK(net.edges_[0]->to_ == net.nodes_[0].get());
  CHECK(net.nodes_[0]->traversals_.at(net.edges_[0].get()) ==
        cr::hash_set<edge*>{net.edges_[0].get()});
}

TEST_CASE("simple_track_eight" * doctest::skip()) {
  auto const level_junction = R"(
# 1 Eight

 *=7=*
 |   |
 |   |
 *===#===*
     |   |
     |   |
     *=8=*

)";
  auto const net = parse_network(level_junction);
  REQUIRE(net.nodes_.size() == 1U);
  REQUIRE(net.edges_.size() == 2U);

  // This can't work!
  // Reason: Level junction with
  // two traversals between the
  // same track pair.
}

TEST_CASE("track_eight_with_level_junctions") {
  auto const level_junction = R"(
# 1 Eight With Level Junctions
     \  |  /
  *=1=#=#=#=1=*
  |    \|/    |
 =#=====#=====#===8=
  |    /|\    |
  *=1=#=#=#=1=*
     /  |  \
)";
  auto const net = parse_network(level_junction);
  REQUIRE(net.nodes_.size() == 9);
  REQUIRE(net.edges_.size() == 24);
}

TEST_CASE("simple_vertical_switch") {
  auto const level_junction = R"(
# 1 TR Switch
 \
  *=
  |\

# 2 T Switch
  |
  *
 /|\

# 3 TR Switch
    /
  =*
  /|

# 4 R Switch
  \
  =*=
  /

# 5 BR Switch
  \|
  =*
    \

# 6 B Switch
 \|/
  *
  |

# 7 BL Switch
  |/
  *=
 /

# 8 R Switch
   /
 =*=
   \
)";
  auto const net = parse_network(level_junction);
  REQUIRE(net.nodes_.size() == 8U);
  for (auto const& n : net.nodes_) {
    for (auto const& [n, targets] : n->traversals_) {
      auto const good = targets.size() == 3 || targets.size() == 1;
      CHECK(good);
    }
  }
  REQUIRE(net.edges_.size() == 8 * 4U);
  for (auto const& e : net.edges_) {
    CHECK(e->dist_ == 1U);
  }
}

TEST_CASE("vertical_switch_with_level_junction") {
  auto const s = R"(
# 1 Vertical Switch

  |
  *
  |\
  | \
  |  \
  |   *
  |   |
 =#===#=
  |   |

)";
  auto const net = parse_network(s);
  CHECK(net.nodes_.size() == 3U);
  CHECK(net.edges_.size() == 8U);
  CHECK(net.edges_[0]->dist_ == 1U);
}

TEST_CASE("level_junction_diagonal") {
  auto const s = R"(
# 1 Level Junction Diagonal
=1======*=
       /
=2====#===
     /
=3==*=====
)";
  auto const net = parse_network(s);
  (void)net;
}

TEST_CASE("level_junction_straight") {
  auto const s = R"(
# 1 Level Junction Straight
   |
   1
   |
=2=#==
   |
=3=#==
   |
=4=#==
   |
)";
  auto const net = parse_network(s);
  (void)net;
}

TEST_CASE("double_slip") {
  auto const s = R"(
# 2 Double Slip Switch

=1==*===*=
     \ /
      #
     / \
=2==*===*=

)";
  auto const net = parse_network(s);
  (void)net;
}

TEST_CASE("single_slip_top") {
  auto const s = R"(
# 2 Single Slip Switch
=1==*   *==
     \ /
      #
     / \
=2==*===*==
)";
  auto const net = parse_network(s);
  (void)net;
}

TEST_CASE("single_slip_top_rl") {
  auto const s = R"(
# 3 Single Slip Alternative
          *=1=
         /
 =2=====^===3=
       /
 =4===*
)";
  auto const net = parse_network(s);

  REQUIRE(net.edges_.size() == 4U);
  REQUIRE(net.nodes_.size() == 1U);

  auto const n = net.nodes_[0].get();
  auto const top = net.edges_[0].get();
  auto const left = net.edges_[1].get();
  auto const right = net.edges_[2].get();
  auto const bottom = net.edges_[3].get();

  CHECK(top->id_ == 1);
  CHECK(left->id_ == 2);
  CHECK(right->id_ == 3);
  CHECK(bottom->id_ == 4);

  CHECK(n->traversals_[top] == cr::hash_set<edge*>{bottom, left});
  CHECK(n->traversals_[bottom] == cr::hash_set<edge*>{top});
  CHECK(n->traversals_[left] == cr::hash_set<edge*>{right, top});
  CHECK(n->traversals_[right] == cr::hash_set<edge*>{left});
}

TEST_CASE("single_slip_top_lr") {
  auto const s = R"(
# 3 Single Slip Alternative
 =1=*
     \
 =2===^=====
       \
        *===
)";

  auto const net = parse_network(s);

  REQUIRE(net.edges_.size() == 4U);
  REQUIRE(net.nodes_.size() == 1U);

  auto const n = net.nodes_[0].get();
  auto const top = net.edges_[0].get();
  auto const left = net.edges_[1].get();
  auto const right = net.edges_[2].get();
  auto const bottom = net.edges_[3].get();

  CHECK(n->traversals_[top] == cr::hash_set<edge*>{bottom, right});
  CHECK(n->traversals_[bottom] == cr::hash_set<edge*>{top});
  CHECK(n->traversals_[left] == cr::hash_set<edge*>{right});
  CHECK(n->traversals_[right] == cr::hash_set<edge*>{left, top});
}

TEST_CASE("single_slip_top_rl_inv") {
  auto const s = R"(
# 3 Single Slip Alternative
          *=1=
         /
 =2=====v===3=
       /
 =4===*
)";
  auto const net = parse_network(s);

  REQUIRE(net.edges_.size() == 4U);
  REQUIRE(net.nodes_.size() == 1U);

  auto const n = net.nodes_[0].get();
  auto const top = net.edges_[0].get();
  auto const left = net.edges_[1].get();
  auto const right = net.edges_[2].get();
  auto const bottom = net.edges_[3].get();

  CHECK(top->id_ == 1);
  CHECK(left->id_ == 2);
  CHECK(right->id_ == 3);
  CHECK(bottom->id_ == 4);

  CHECK(n->traversals_[top] == cr::hash_set<edge*>{bottom});
  CHECK(n->traversals_[bottom] == cr::hash_set<edge*>{top, right});
  CHECK(n->traversals_[left] == cr::hash_set<edge*>{right});
  CHECK(n->traversals_[right] == cr::hash_set<edge*>{left, top});
}

TEST_CASE("single_slip_top_lr_inv") {
  auto const s = R"(
# 3 Single Slip Alternative
 =1=*
     \
 =2===v=====
       \
        *===
)";

  auto const net = parse_network(s);

  REQUIRE(net.edges_.size() == 4U);
  REQUIRE(net.nodes_.size() == 1U);

  auto const n = net.nodes_[0].get();
  auto const top = net.edges_[0].get();
  auto const left = net.edges_[1].get();
  auto const right = net.edges_[2].get();
  auto const bottom = net.edges_[3].get();

  CHECK(n->traversals_[top] == cr::hash_set<edge*>{bottom});
  CHECK(n->traversals_[bottom] == cr::hash_set<edge*>{top, left});
  CHECK(n->traversals_[left] == cr::hash_set<edge*>{right, bottom});
  CHECK(n->traversals_[right] == cr::hash_set<edge*>{left});
}

TEST_CASE("mixed_single_slip_level_junction") {
  auto const s = R"(
# 3 Mix

            *=1=
            |
            *
           /|
          2 3
         /  |
 =4=====^=5=#==6==
       /    |
      7     8
     /      |
 =9=*====10=#==11=
            |
            |
            |
   ====12===*
)";
  auto const net = parse_network(s);
  REQUIRE(net.nodes_.size() == 5);
  REQUIRE(net.edges_.size() == 12);
}

TEST_CASE("horizontal_signal") {
  auto const net1 = parse_network(R"(
# 3 Horizantal Signal

 =1=AA>==2==<FF=3=

)");
  auto const net2 = parse_network(R"(
# 3 Vertical Signal

 |
 1
 |
 A
 A
 !
 |
 2
 |
 ;
 F
 F
 |
 3
 |

)");

  for (auto const& n : {&net1, &net2}) {
    auto const& net = *n;

    REQUIRE(net.nodes_.size() == 2);
    REQUIRE(net.edges_.size() == 3);

    auto const s1 = net.nodes_[0].get();
    auto const s2 = net.nodes_[1].get();
    auto const e1 = net.edges_[0].get();
    auto const e2 = net.edges_[1].get();
    auto const e3 = net.edges_[2].get();

    CHECK(s1->name_ == "AA");
    CHECK(s2->name_ == "FF");
    CHECK(e1->id_ == 1);
    CHECK(e2->id_ == 2);
    CHECK(e3->id_ == 3);

    CHECK(s1->traversals_.size() == 2);
    CHECK(s1->traversals_[e1] == cr::hash_set<edge*>{e2});
    CHECK(s1->traversals_[e2] == cr::hash_set<edge*>{e1});
    CHECK(s1->action_traversal_ == std::pair{e1, e2});

    CHECK(s2->traversals_.size() == 2);
    CHECK(s2->traversals_[e2] == cr::hash_set<edge*>{e3});
    CHECK(s2->traversals_[e3] == cr::hash_set<edge*>{e2});
    CHECK(s2->action_traversal_ == std::pair{e3, e2});
  }
}

TEST_CASE("approach_signal_signal_end_of_train_detector") {
  auto const net = parse_network(R"(
# 1 Approach Signal -> Signal -> End of Train Detector

 a==1===)======2======AA>=====3==========]==4==b

)");

  REQUIRE(net.nodes_.size() == 5);
  REQUIRE(net.edges_.size() == 4);

  auto const e0 = net.edges_[0].get();
  auto const e1 = net.edges_[1].get();
  auto const e2 = net.edges_[2].get();
  auto const e3 = net.edges_[3].get();

  CHECK(e0->id_ == 1);
  CHECK(e1->id_ == 2);
  CHECK(e2->id_ == 3);
  CHECK(e3->id_ == 4);

  auto const n0 = net.nodes_[1].get();
  auto const n1 = net.nodes_[2].get();
  auto const n2 = net.nodes_[3].get();

  CHECK(n0->type_ == node::type::APPROACH_SIGNAL);
  CHECK(n1->type_ == node::type::MAIN_SIGNAL);
  CHECK(n2->type_ == node::type::END_OF_TRAIN_DETECTOR);

  CHECK(n0->traversals_.at(e0) == cr::hash_set<edge*>{e1});
  CHECK(n0->traversals_.at(e1) == cr::hash_set<edge*>{e0});
  CHECK(n0->action_traversal_ == std::pair{e0, e1});

  CHECK(n1->traversals_.at(e1) == cr::hash_set<edge*>{e2});
  CHECK(n1->traversals_.at(e2) == cr::hash_set<edge*>{e1});
  CHECK(n1->action_traversal_ == std::pair{e1, e2});

  CHECK(n2->traversals_.at(e2) == cr::hash_set<edge*>{e3});
  CHECK(n2->traversals_.at(e3) == cr::hash_set<edge*>{e2});
  CHECK(n2->action_traversal_ == std::pair{e2, e3});
}

TEST_CASE("station") {
  parse_network(R"(
# 1 Simple Double Track Station
                                 *=<Q=====O>=*
                                /             \
=)=====[=====AA>=)=*===*=======*===<P=====L>===*=*=========*=(=<F====]=======(==
                    \ /                           \       /
                     X                             \     /
                    / \                             \   /
=)=====[======A>=)=*===*=======*===<S====N>====*=====*=*=====(=<FF===]=======(==
                                \             /
                                 *=<R====M>==*
)");
}
