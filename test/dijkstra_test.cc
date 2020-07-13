#include "doctest/doctest.h"

#include "rapid/ascii_network_parser.h"
#include "rapid/dijkstra.h"

using namespace rapid;

TEST_CASE("dijkstra_1") {
  auto const net = parse_network(R"(
# 1 Simple Double Track Station
                                   *=<Q========*
                                  /             \
 a=)=====[=====AA>===*===*=======*===<P==========*=*=========*==<F===]=====(==j
                      \ /                           \       /
                       #                             \     /
                      / \                             \   /
 i=)=====[======A>===*===*=======*=========N>====*=====*=*======<FF==]=====(==b
                                  \             /
                                   *=======M>==*
)");

  CHECK(dijkstra(net, "j", "Q").size() != 0U);
  CHECK(dijkstra(net, "Q", "a").size() != 0U);
  CHECK(dijkstra(net, "i", "j").size() != 0U);
  CHECK(dijkstra(net, "a", "j").size() != 0U);
  CHECK(dijkstra(net, "FF", "AA").size() != 0U);
  CHECK(dijkstra(net, "b", "P").size() != 0U);
}

TEST_CASE("dijkstra_node_per_incoming_edge_test") {
  auto const net = parse_network(R"(
# 1 Sepearate Nodes Test

           *============*
      *=*  |            |
      | |  |            |
      | *==#==FF>=======*
      |    |
      *====*
     /
  a=*

)");
  CHECK(dijkstra(net, "a", "FF").size() != 0U);
}