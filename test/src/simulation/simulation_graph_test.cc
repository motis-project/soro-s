#include "doctest/doctest.h"

#include "soro/utls/std_wrapper/accumulate.h"

#include "soro/simulation/simulation_graph.h"

#include "test/file_paths.h"

namespace soro::simulation::test {

using namespace soro::tt;
using namespace soro::infra;

soro::size_t get_total_train_dependencies(simulation_graph const& sg) {
  return utls::accumulate(
      sg.train_dependencies_, soro::size_t{0},
      [](auto&& acc, auto&& bucket) { return acc + bucket.size(); });
}

soro::size_t get_total_timetable_dependencies(simulation_graph const& sg) {
  return utls::accumulate(
      sg.timetable_dependencies_, soro::size_t{0},
      [](auto&& acc, auto&& train_dep) { return acc + train_dep.has_value(); });
}

TEST_SUITE("simulation graph") {
  TEST_CASE("simulation graph, follow") {
    auto opts = soro::test::SMALL_OPTS;
    auto tt_opts = soro::test::FOLLOW_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.exclusion_elements_ = false;
    opts.exclusion_graph_ = false;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    ordering_graph const og(infra, tt);
    simulation_graph const sg(infra, tt, og);

    auto const total_train_dependencies = get_total_train_dependencies(sg);
    CHECK_EQ(total_train_dependencies, 5);

    auto const total_tt_dependencies = get_total_timetable_dependencies(sg);
    CHECK_EQ(total_tt_dependencies, 6);
  }

  TEST_CASE("ordering graph, cross") {
    auto opts = soro::test::SMALL_OPTS;
    auto tt_opts = soro::test::CROSS_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.exclusion_graph_ = false;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    ordering_graph const og(infra, tt);
    simulation_graph const sg(infra, tt, og);

    auto const total_train_dependencies = get_total_train_dependencies(sg);
    CHECK_EQ(total_train_dependencies, 2);

    auto const total_tt_dependencies = get_total_timetable_dependencies(sg);
    CHECK_EQ(total_tt_dependencies, 6);
  }

  TEST_CASE("de_kss graph, simulation") {
    auto opts = soro::test::DE_ISS_OPTS;
    auto tt_opts = soro::test::DE_KSS_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.layout_ = false;
    opts.exclusion_elements_ = false;
    opts.exclusion_graph_ = false;

    interval const inter{.start_ = rep_to_absolute_time(1636786800),
                         .end_ = rep_to_absolute_time(1636786800) + hours{2}};

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    ordering_graph const og(infra, tt, {.interval_ = inter});
    simulation_graph const sg(infra, tt, og);
  }
}

}  // namespace soro::simulation::test
