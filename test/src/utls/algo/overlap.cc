#include "doctest/doctest.h"

#include <vector>

#include "soro/utls/algo/overlap.h"

using namespace soro::utls;

TEST_SUITE("overlap suite") {
  TEST_CASE("overlap none") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const v2 = {8, 9, 10, 11, 12, 13};

    CHECK(!overlap(v1, v2));
  }

  TEST_CASE("overlap some") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const v2 = {-5, -4, -3, 1, 91, 92, 93};

    CHECK(overlap(v1, v2));
  }

  TEST_CASE("overlap all") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const& v2 = v1;

    CHECK(overlap(v1, v2));
  }
}

TEST_SUITE("overlap_non_sorted suite") {
  TEST_CASE("overlap_non_sorted none") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const v2 = {8, 9, 10, 11, 12, 13};

    auto result1 =
        overlap_non_sorted(v1.cbegin(), v1.cend(), v2.cbegin(), v2.cend());
    CHECK(!result1);

    auto result2 = overlap_non_sorted(v1, v2);
    CHECK(!result2);
  }

  TEST_CASE("overlap_non_sorted some") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const v2 = {-5, -4, -3, 1, 91, 92, 93};

    auto result1 =
        overlap_non_sorted(v1.cbegin(), v1.cend(), v2.cbegin(), v2.cend());
    CHECK(result1);

    auto result2 = overlap_non_sorted(v1, v2);
    CHECK(result2);
  }

  TEST_CASE("overlap_non_sorted all") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const& v2 = v1;

    auto result1 =
        overlap_non_sorted(v1.cbegin(), v1.cend(), v2.cbegin(), v2.cend());
    CHECK(result1);

    auto result2 = overlap_non_sorted(v1, v2);
    CHECK(result2);
  }
}

TEST_SUITE("get_overlap suite") {
  TEST_CASE("get_overlap none") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const v2 = {8, 9, 10, 11, 12, 13};

    auto const x = get_overlap(v1, v2);
    CHECK(get_overlap(v1, v2).empty());
  }

  TEST_CASE("get_overlap some") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7, 10, 10, 13, 93};
    std::vector<int> const v2 = {-5, -4, -3, 1, 2, 10, 10, 13, 91, 92, 93};

    std::vector<int> const expected = {1, 2, 10, 10, 13, 93};

    auto const result = get_overlap(v1, v2);
    CHECK(result == expected);
  }

  TEST_CASE("get_overlap all") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const& v2 = v1;

    CHECK(get_overlap(v1, v2) == v1);
  }
}

TEST_SUITE("get_overlap_non_sorted suite") {
  TEST_CASE("get_overlap_non_sorted none") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> const v2 = {8, 9, 10, 11, 12, 13};

    auto const x = get_overlap_non_sorted(v1, v2);
    CHECK(get_overlap(v1, v2).empty());
  }

  TEST_CASE("get_overlap_non_sorted some") {  // NOLINT
    std::vector<int> const v1 = {1, 2, 3, 4, 5, 6, 93, 10, 10, 11, 11, 13};
    std::vector<int> const v2 = {-5, -4, -3, 1, 2, 10, 10, 13, 91, 11, 92, 93};

    std::vector<int> const expected = {1, 2, 93, 10, 10, 11, 13};

    auto const result = get_overlap_non_sorted(v1, v2);
    CHECK(result == expected);
  }

  TEST_CASE("get_overlap_non_sorted all") {  // NOLINT
    std::vector<int> const v1 = {3, -10, 11, 4, 1000, 6, 0};
    std::vector<int> const& v2 = v1;

    CHECK(get_overlap_non_sorted(v1, v2) == v1);
  }
}
