#include "doctest/doctest.h"

#include "rapid/ascii_network_parser.h"
#include "rapid/timetable_parser.h"

using namespace rapid;

TEST_CASE("timetable_parser") {
  auto const net =
      parse_network("a===)=A>=]=====)=B>=]===)=C>=]=====)=D>=]===b");
  auto const tt = parse_timetable(net, R"(TRAIN,SPEED
X,3
Y,6
Z,9
)",
                                  R"(TRAIN,POSITION,TIME
X,a,2020-01-01 13:00:00
X,b,2020-01-01 13:05:00
Y,a,2020-01-01 13:02:00
Y,b,2020-01-01 13:03:30
Z,a,2020-01-01 13:02:30
Z,b,2020-01-01 13:04:10
)");

  for (auto const& [name, t] : tt) {
    CHECK(t->timetable_.size() == 2U);
  }
}