#include "doctest/doctest.h"

#include "utl/enumerate.h"

#include "soro/infrastructure/exclusion/get_exclusion_graph.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "test/file_paths.h"

using namespace soro::infra;

TEST_CASE("exclusion graph is symmetric") {
  auto opts = soro::test::SMALL_OPTS;
  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.layout_ = false;

  infrastructure const infra(opts);

  auto const g = get_exclusion_graph(infra);

  for (auto const [from, tos] : utl::enumerate(g.nodes_)) {
    for (auto const to : tos) {
      CHECK(g.nodes_[to][static_cast<interlocking_route::id>(from)]);
    }
  }
}