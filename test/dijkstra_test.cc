#include <iostream>

#include "doctest/doctest.h"

#include "rapid/ascii_network_parser.h"
#include "rapid/dijkstra.h"

using namespace rapid;

TEST_CASE("dijkstra") {
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

  net.print(dijkstra(net, "j", "Q"));
  net.print(dijkstra(net, "Q", "a"));
  net.print(dijkstra(net, "i", "j"));
  net.print(dijkstra(net, "a", "j"));
  net.print(dijkstra(net, "FF", "AA"));
  net.print(dijkstra(net, "b", "P"));
}

TEST_CASE("dijkstra") {
  auto const net = parse_network(R"(
# 1 Simple Double Track Station

           *============*
      *=*  |            |
      | |  |            |
      | *==#==FF>=======*
      |    |
      *====*
     /
  a=*

)");

  net.print(dijkstra(net, "a", "FF"));
}