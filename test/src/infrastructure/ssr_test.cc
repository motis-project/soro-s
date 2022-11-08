#include "doctest/doctest.h"

#include "test/file_paths.h"

#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/is_path.h"

using namespace soro;
using namespace infra;

TEST_SUITE("signal station route suite") {

  TEST_CASE("signal station route exclusion") {  // NOLINT
  }

  TEST_CASE("interlocking routes are paths") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);

    for (auto const& ir : infra->interlocking_.interlocking_routes_) {
      CHECK(is_path(utls::coro_map(ir->entire(skip_omitted::OFF), [](auto&& r) {
        return r.node_->element_;
      })));

      CHECK(is_path(ir->nodes()));
    }
  }
}