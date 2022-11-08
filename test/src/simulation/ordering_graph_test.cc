#include "doctest/doctest.h"

#include "soro/simulation/ordering_graph.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;
using namespace soro::simulation;

TEST_SUITE("ordering graph") {
  TEST_CASE("ordering graph - follow") {
    auto const infra = infrastructure(SMALL_OPTS);
    auto const tt = timetable(FOLLOW_OPTS, infra);

    ordering_graph const og(infra, tt);
  }
}
