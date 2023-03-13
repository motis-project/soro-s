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
  using namespace date;
  //  auto opts = soro::test::SMALL_OPTS;
  //  auto tt_opts = soro::test::FOLLOW_OPTS;
  auto opts = soro::test::DE_ISS_OPTS;
  auto tt_opts = soro::test::DE_KSS_OPTS;

  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.exclusion_graph_ = false;
  opts.layout_ = false;

  infrastructure const infra(opts);

  timetable const tt(tt_opts, infra);

  interval const one_week = {.start_ = ymd_to_abs(2021_y / August / 1),
                             .end_ = ymd_to_abs(2021_y / August / 8)};

  ordering_graph const og1(infra, tt, one_week);

  interval const one_day = {.start_ = ymd_to_abs(2021_y / August / 1),
                            .end_ = ymd_to_abs(2021_y / August / 2)};

  ordering_graph const og2(infra, tt, one_day);

  interval const two_hours = {
      .start_ = ymd_to_abs(2021_y / August / 1) + hours{10},
      .end_ = ymd_to_abs(2021_y / August / 1) + hours{12}};

  ordering_graph const og3(infra, tt, two_hours);
 
  ordering_graph const og4(infra, tt, interval{});

  //  std::ignore = og;
}
