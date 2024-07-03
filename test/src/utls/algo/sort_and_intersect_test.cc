#include "doctest/doctest.h"

#include "utl/zip.h"

#include "soro/base/soro_types.h"

#include "soro/utls/algo/sort_and_intersect.h"

TEST_SUITE("sort and intersect suite") {

  using namespace soro::utls;

  TEST_CASE("sort and intersect simple") {  // NOLINT
    soro::vector<int> a = {9, 3, 4, 5, 1};
    soro::vector<int> b = {3, 4, 5, 6, 7, 10, 1};

    soro::vector<int> const expected = {1, 3, 4, 5};
    soro::vector<int> const result = sort_and_intersect(a, b);

    CHECK_EQ(expected.size(), result.size());

    for (auto const [e, r] : utl::zip(expected, result)) {
      CHECK_EQ(e, r);
    }
  }

  TEST_CASE("sort and intersect empty") {  // NOLINT
    soro::vector<int> a = {9, 3, 4, 5, 1};
    soro::vector<int> b = {6, 8, 8, 6, 7, 10, 0};

    soro::vector<int> const expected = {};
    soro::vector<int> const result = sort_and_intersect(a, b);

    CHECK_EQ(expected.size(), result.size());
  }

  TEST_CASE("sort and intersect doubles") {  // NOLINT
    soro::vector<int> a = {3, 9, 3, 4, 5, 1, 1, 1};
    soro::vector<int> b = {3, 4, 5, 6, 4, 7, 5, 10, 1};

    soro::vector<int> const expected = {1, 3, 4, 5};
    soro::vector<int> const result = sort_and_intersect(a, b);

    CHECK_EQ(expected.size(), result.size());

    for (auto const [e, r] : utl::zip(expected, result)) {
      CHECK_EQ(e, r);
    }
  }

}  // sort_and_intersect suite
