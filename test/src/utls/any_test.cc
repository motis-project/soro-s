#include "doctest/doctest.h"

#include <array>

#include "soro/utls/any.h"

namespace soro::utls {

static_assert(0 == any{0, 1});
static_assert(3 != any{0, 1});

static_assert(1 == any{std::array<int, 5>{1, 2, 3, 4, 5}});
static_assert(0 != any{std::array<int, 5>{1, 2, 3, 4, 5}});

static_assert([] {
  std::array<int, 5> const a{1, 2, 3, 4, 5};
  return 1 == any{a};
}());

static_assert([] {
  std::array<int, 5> const a{1, 2, 3, 4, 5};
  return 0 != any{a};
}());

static_assert([] -> bool {
  constexpr std::array<int, 5> a{0, 1, 2, 3, 4};
  auto result = 1 == any{a};
  static_assert(a[4] == 4);
  return result;
}());

static_assert([] -> bool {
  constexpr std::array<int, 5> a{0, 1, 2, 3, 4};
  auto result = 5 != any{a};
  static_assert(a[4] == 4);
  return result;
}());

TEST_SUITE("any") {
  TEST_CASE("t1") {
    const constexpr int i = 0;
    const constexpr int i1 = 1;
    const constexpr int i2 = 2;
    std::array<int, 2> constexpr es{i1, i2};

    static_assert(i != any{es});
    static_assert(i1 == any{es});
    static_assert(i != any{std::array<int, 2>{i1, i2}});
    CHECK((i + 1 == any{es}));
  }
}

}  // namespace soro::utls
