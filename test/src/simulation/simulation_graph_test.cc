#include "doctest/doctest.h"

#include "soro/simulation/simulation_graph.h"

#include "test/file_paths.h"

namespace soro::simulation::test {

using namespace soro::tt;
using namespace soro::infra;

TEST_SUITE("simulation graph") {

  TEST_CASE("simulation graph, follow") {
    auto opts = soro::test::SMALL_OPTS;
    auto tt_opts = soro::test::FOLLOW_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.exclusion_graph_ = true;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    ordering_graph const og(infra, tt);
    simulation_graph const sg(infra, tt, og);

    std::cout << sg.nodes_.size() << '\n';
  }

  TEST_CASE("de_kss graph" * doctest::skip(true)) {
    auto opts = soro::test::DE_ISS_OPTS;
    auto tt_opts = soro::test::DE_KSS_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.exclusion_graph_ = true;
    opts.layout_ = false;

    interval const inter{.start_ = rep_to_absolute_time(1636786800),
                         .end_ = rep_to_absolute_time(1636786800) + hours{2}};

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    ordering_graph const og(infra, tt, {.interval_ = inter});
    simulation_graph const sg(infra, tt, og);
  }
}

}  // namespace soro::simulation::test
