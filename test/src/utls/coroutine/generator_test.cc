#include "doctest/doctest.h"

#include <vector>

#include "soro/utls/coroutine/generator.h"

using namespace soro::utls;

TEST_SUITE("generator suite") {
  template <typename T>
  generator<const T> generate_vec(std::vector<T> const& v) {
    for (auto const& e : v) {
      co_yield(e);
    }
    co_yield 11;
  }

  TEST_CASE("generator simple") {
    std::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::vector<int> result;
    for (auto const& i : generate_vec(v)) {
      result.emplace_back(i);
    }

    std::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    CHECK(result == expected);
  }
}