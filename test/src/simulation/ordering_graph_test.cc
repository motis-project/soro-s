#include "doctest/doctest.h"

#include "soro/simulation/ordering_graph.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;
using namespace soro::simulation;

namespace soro::simulation::test {

TEST_CASE("ordering graph - follow" * doctest::skip()) {
  for (auto const& scenario : soro::test::get_timetable_scenarios()) {
    ordering_graph const og(scenario.infra_, scenario.timetable_);
  }
}

}  // namespace soro::simulation::test
