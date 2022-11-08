#include "doctest/doctest.h"

#include <vector>

#include "soro/utls/algo/sort_and_intersect.h"

#include "utl/zip.h"

TEST_SUITE("sort and intersect suite") {

  using namespace soro::utls;

  TEST_CASE("sort and intersect simple") {  // NOLINT
    std::vector<int> a = {9, 3, 4, 5, 1};
    std::vector<int> b = {3, 4, 5, 6, 7, 10, 1};

    std::vector<int> const expected = {1, 3, 4, 5};
    std::vector<int> const result = sort_and_intersect(a, b);

    CHECK(expected.size() == result.size());

    for (auto const [e, r] : utl::zip(expected, result)) {
      CHECK(e == r);
    }
  }

  TEST_CASE("sort and intersect empty") {  // NOLINT
    std::vector<int> a = {9, 3, 4, 5, 1};
    std::vector<int> b = {6, 8, 8, 6, 7, 10, 0};

    std::vector<int> const expected = {};
    std::vector<int> const result = sort_and_intersect(a, b);

    CHECK(expected.size() == result.size());
  }

  TEST_CASE("sort and intersect doubles") {  // NOLINT
    std::vector<int> a = {3, 9, 3, 4, 5, 1, 1, 1};
    std::vector<int> b = {3, 4, 5, 6, 4, 7, 5, 10, 1};

    std::vector<int> const expected = {1, 3, 4, 5};
    std::vector<int> const result = sort_and_intersect(a, b);

    CHECK(expected.size() == result.size());

    for (auto const [e, r] : utl::zip(expected, result)) {
      CHECK(e == r);
    }
  }

}  // sort_and_intersect suite
