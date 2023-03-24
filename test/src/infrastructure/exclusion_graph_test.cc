#include "doctest/doctest.h"

#include "utl/enumerate.h"

#include "test/file_paths.h"

using namespace soro::infra;

TEST_CASE("exclusion graph is symmetric") {
  auto opts = soro::test::SMALL_OPTS;
  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.exclusion_graph_ = true;
  opts.layout_ = false;

  infrastructure const infra(opts);
  auto const& g = infra->exclusion_.exclusion_graph_;

  for (auto const [from, tos] : utl::enumerate(g.nodes_)) {
    for (auto const to : tos) {
      CHECK(g.nodes_[to][static_cast<interlocking_route::id>(from)]);
    }
  }
}