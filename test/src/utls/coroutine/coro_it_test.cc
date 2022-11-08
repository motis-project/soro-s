#include "doctest/doctest.h"

#include <vector>

#include "soro/utls/coroutine/iterator.h"

using namespace soro::utls;

TEST_SUITE("coro_it") {
  TEST_CASE("coro_it range based for loop") {
    struct range_based {
      coro_it<int> begin() {
        for (auto i : v_) {
          co_yield i;
        }
        co_yield 11;
      }

      coro_it<int> end() { return {}; }  // NOLINT

      int i_{11};
      std::vector<int> v_ = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    };

    range_based rb;

    std::vector<int> result;
    for (auto i : rb) {
      result.emplace_back(i);
    }

    auto b = rb.begin();

    std::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    CHECK(result == expected);
  }

  TEST_CASE("coro_it range based for loop const ref") {
    struct range_based {
      coro_it<int const&> begin() {
        for (auto const& i : v_) {
          co_yield i;
        }
        co_yield i_;
      }

      coro_it<int const&> end() { return {}; }  // NOLINT

      int i_{11};
      std::vector<int> v_ = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    };

    range_based rb;

    std::vector<int> result;
    for (auto const& i : rb) {
      result.emplace_back(i);
    }

    std::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    CHECK(result == expected);
  }

  TEST_CASE("coro_it range based for loop ref") {
    struct range_based {
      coro_it<int&> begin() {
        for (auto& i : v_) {
          co_yield i;
        }
        co_yield i_;
      }

      coro_it<int&> end() { return {}; }  // NOLINT

      int i_{11};
      std::vector<int> v_ = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    };

    range_based rb;

    std::vector<int> result;
    for (auto& i : rb) {
      ++i;
      result.emplace_back(i);
    }

    std::vector<int> const expected = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    CHECK(result == expected);

    std::vector<int> const rb_expected = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    CHECK(rb.v_ == rb_expected);
    CHECK(rb.i_ == 12);
  }
}
