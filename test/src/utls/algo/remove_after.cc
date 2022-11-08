#include "doctest/doctest.h"

#include "soro/utls/algo/remove_after.h"

TEST_SUITE("remove after suite") {

  using namespace soro::utls;

  TEST_CASE("remove after - take all") {  // NOLINT
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    remove_after(v, [](auto&& i) { return i == 10; });

    std::vector<int> const expected = v;
    CHECK(v == expected);
  }

  TEST_CASE("remove after - take some") {  // NOLINT
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    remove_after(v, [](auto&& i) { return i == 3; });

    std::vector<int> const expected = {1, 2, 3};
    CHECK(v == expected);
  }

  TEST_CASE("remove after - take one") {  // NOLINT
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    remove_after(v, [](auto&& i) { return i == 1; });

    std::vector<int> const expected = {1};
    CHECK(v == expected);
  }
}
