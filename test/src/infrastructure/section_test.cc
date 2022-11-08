#include "doctest/doctest.h"

#include "test/file_paths.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/is_path.h"

using namespace soro;
using namespace infra;

TEST_SUITE("infrastructure section") {
  TEST_CASE("sections are paths") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);

    for (auto const& sec : infra->graph_.sections_) {
      CHECK(is_path(sec.iterate(rising::YES)));
      CHECK(is_path(sec.iterate(rising::NO)));
    }
  }
}
