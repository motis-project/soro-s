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
    ordering_graph const og(*scenario->infra_, scenario->timetable_);
  }
}

}  // namespace soro::simulation::test

TEST_CASE("o") {
  auto opts = soro::test::SMALL_OPTS;
  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.exclusion_graph_ = false;
  opts.layout_ = false;

  infrastructure const infra(opts);

  auto tt_opts = soro::test::FOLLOW_OPTS;

  timetable const tt(tt_opts, infra);

  for (auto const& train : tt->trains_) {
    for (auto const midnight : train.departures()) {
      //      fmt::print(std::cout, "midnight {}", midnight);
      std::cout << "midnidhgt " << midnight.time_since_epoch().count() << '\n';
    }
  }

  //  ordering_graph const og(infra, tt);

  //  std::ignore = og;
}
