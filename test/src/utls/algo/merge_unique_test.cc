#include "doctest/doctest.h"

#include <vector>

#include "utl/zip.h"

#include "soro/utls/algo/merge_unique.h"

using namespace soro::utls;

TEST_SUITE("merge_unique suite") {

  TEST_CASE("merge_unique simple") {  // NOLINT
    std::vector<int> const a = {1, 2, 3, 4, 5};
    std::vector<int> const b = {3, 4, 5, 6, 7};

    std::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const result = merge_unique(a, b);

    CHECK(expected.size() == result.size());

    for (auto const [e, r] : utl::zip(expected, result)) {
      CHECK(e == r);
    }
  }

  TEST_CASE("merge_unique empty") {  // NOLINT
    std::vector<int> const a = {};
    std::vector<int> const b = {};

    std::vector<int> const expected = {};
    std::vector<int> const result = merge_unique(a, b);

    CHECK(expected.size() == result.size());
  }

#if !(defined(__EMSCRIPTEN__) || defined(SORO_SAN))
  TEST_CASE("merge_unique non sorted throws") {  // NOLINT
    std::vector<int> const a = {1, 2, 3};
    std::vector<int> const b = {3, 2, 1};

    CHECK_THROWS(merge_unique(a, b));
  }
#endif

}  // merge_unique suite
