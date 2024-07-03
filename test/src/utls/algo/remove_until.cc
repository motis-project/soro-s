#include "doctest/doctest.h"

#include <vector>

#include "soro/utls/algo/remove_until.h"

TEST_SUITE("remove until suite") {

  using namespace soro::utls;

  TEST_CASE("remove until - take all") {  // NOLINT
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    remove_until(v, [](auto&& i) { return i == 10; });

    std::vector<int> const expected = v;
    CHECK(v == expected);
  }

  TEST_CASE("remove until - take some") {  // NOLINT
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    remove_until(v, [](auto&& i) { return i == 3; });

    std::vector<int> const expected = {3, 4, 5, 6, 7};
    CHECK(v == expected);
  }

  TEST_CASE("remove until - take one") {  // NOLINT
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    remove_until(v, [](auto&& i) { return i == 7; });

    std::vector<int> const expected = {7};
    CHECK(v == expected);
  }
}
