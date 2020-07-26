#include "doctest/doctest.h"

#include <iostream>

#include "soro/ascii_network_parser.h"
#include "soro/graphviz_output.h"
#include "soro/propagator.h"
#include "soro/route_train_order.h"
#include "soro/timetable_parser.h"

using namespace soro;

struct r {
  cista::raw::hash_set<route*> in_;
};

TEST_CASE("timetable_parser_1") {
  auto const net =
      parse_network("a===)=A>=]======)=B>=]======)=C>=]======)=D>=]===b");
  auto const tt = parse_timetable(net, R"(TRAIN,SPEED
X,50
Y,100
Z,200
)",
                                  R"(TRAIN,POSITION,TIME
X,a,2020-01-01 13:00:00
X,B,2020-01-01 13:08:00
X,b,2020-01-01 14:09:00
Y,a,2020-01-01 13:32:00
Y,B,2020-01-01 13:46:00
Y,b,2020-01-01 14:07:00
Z,a,2020-01-01 13:49:00
Z,b,2020-01-01 14:07:00
)");

  compute_route_train_order(tt);
  propagate(tt);
  for (auto const& [train_name, train] : tt) {
    std::cout << *train << "\n";
  }
  graphiz_output(std::cout, tt);
}

TEST_CASE("timetable_parser_2") {
  auto const net = parse_network(R"(
=a=)==U>=]=========)==A>=]==*   *=)===D>=]===)==Y>==]=d=
                             \ /
                              X
                             / \
=c=)==V>=]=========)==C>=]==*   *=)===B>=]===)==Z>==]=b=
)");
  auto const tt = parse_timetable(net, R"(TRAIN,SPEED
X,100
Y,100
)",
                                  R"(TRAIN,POSITION,TIME
X,a,2020-01-01 13:00:00
X,b,2020-01-01 13:08:00
Y,c,2020-01-01 13:01:00
Y,d,2020-01-01 13:08:00
)");

  compute_route_train_order(tt);
  propagate(tt);
  for (auto const& [train_name, train] : tt) {
    std::cout << *train << "\n";
  }
  graphiz_output(std::cout, tt);
}

TEST_CASE("timetable_parser_3") {
  auto const net = parse_network(R"(
           *=[=<C=(====*
          /             \
a=)=A>=]=*               *=[=<D=(=b
          \             /
           *===)=B>=]==*
)");
  auto const tt = parse_timetable(net, R"(TRAIN,SPEED
X,100
Y,100
)",
                                  R"(TRAIN,POSITION,TIME
X,a,2020-01-01 13:00:00
X,B,2020-01-01 13:01:00
X,b,2020-01-01 13:02:00
Y,b,2020-01-01 13:01:00
Y,C,2020-01-01 13:02:00
Y,a,2020-01-01 13:03:00
)");

  compute_route_train_order(tt);
  propagate(tt);
  for (auto const& [train_name, train] : tt) {
    std::cout << *train << "\n";
  }
  graphiz_output(std::cout, tt);
}
