#include "doctest/doctest.h"

#include <vector>

#include "soro/utls/algo/overlap.h"

using namespace soro::utls;

TEST_SUITE("overlap suite") {

  TEST_CASE("overlap simple true") {  // NOLINT
    std::vector<int> const a = {1, 2, 3, 4, 5};
    std::vector<int> const b = {3, 4, 5, 6, 7};

    CHECK(overlap(a, b));
  }

  TEST_CASE("overlap simple false") {  // NOLINT
    std::vector<int> const a = {1, 2, 3, 4, 5};
    std::vector<int> const b = {6, 7, 8, 9, 10};

    CHECK(!overlap(a, b));
  }

#if !(defined(__EMSCRIPTEN__) || defined(SORO_SAN))
  TEST_CASE("overlap non sorted throws") {  // NOLINT
    std::vector<int> const a = {1, 2, 3, 5, 4, 6};
    std::vector<int> const b = {6, 7, 8, 9, 10};

    CHECK_THROWS(overlap(a, b));
  }
#endif

}  // overlap suite
