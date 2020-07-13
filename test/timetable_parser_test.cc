#include "doctest/doctest.h"

#include <iostream>

#include "utl/pairwise.h"
#include "utl/verify.h"

#include "rapid/ascii_network_parser.h"
#include "rapid/graphviz_output.h"
#include "rapid/propagator.h"
#include "rapid/route_train_order.h"
#include "rapid/timetable_parser.h"

using namespace rapid;

struct r {
  cista::raw::hash_set<route*> in_;
};

TEST_CASE("timetable_parser") {
  auto const net =
      parse_network("a===)=A>=]======)=B>=]======)=C>=]======)=D>=]===b");
  auto const tt = parse_timetable(net, R"(TRAIN,SPEED
X,50
Y,100
Z,200
)",
                                  R"(TRAIN,POSITION,TIME
X,a,2020-01-01 13:00:00
X,b,2020-01-01 14:09:00
Y,a,2020-01-01 13:32:00
Y,b,2020-01-01 14:07:00
Z,a,2020-01-01 13:49:00
Z,b,2020-01-01 14:07:00
)");

  auto const route_train_order = compute_route_train_order(tt);
  propagate(route_train_order);
  for (auto const& [train_name, train] : tt) {
    std::cout << *train << "\n";
  }
  graphiz_output(std::cout, tt);
}