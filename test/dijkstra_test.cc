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

  net.print(std::cout, dijkstra(net, "a", "b"));
  net.print(std::cout, dijkstra(net, "i", "j"));
  net.print(std::cout, dijkstra(net, "a", "j"));
  net.print(std::cout, dijkstra(net, "FF", "AA"));
  net.print(std::cout, dijkstra(net, "b", "P"));
  net.print(std::cout, dijkstra(net, "P", "a"));
}