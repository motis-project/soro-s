#include "doctest/doctest.h"

#include <array>

#include "soro/utls/all.h"

namespace soro::utls {

static_assert(0 == all{0, 0});
static_assert(3 != all{3, 1});

static_assert(1 == all{std::array<int, 5>{1, 1, 1, 1, 1}});
static_assert(1 != all{std::array<int, 5>{1, 1, 1, 1, 2}});

static_assert([] {
  std::array<int, 5> const a{1, 1, 1, 1, 1};
  return 1 == all{a};
}());

static_assert([] {
  std::array<int, 5> const a{1, 2, 3, 4, 5};
  return 0 != all{a};
}());

static_assert([] {
  std::array<int, 5> const a{1, 2, 3, 4, 5};
  return 1 != all{a};
}());

static_assert([] -> bool {
  constexpr std::array<int, 5> a{1, 1, 1, 1, 1};
  auto result = 1 == all{a};
  static_assert(a[4] == 1);
  return result;
}());

static_assert([] -> bool {
  constexpr std::array<int, 5> a{0, 1, 2, 3, 4};
  auto result = 5 != all{a};
  static_assert(a[4] == 4);
  return result;
}());

TEST_SUITE("all") {
  TEST_CASE("t1") {
    const constexpr int i = 0;
    const constexpr int i1 = 1;
    const constexpr int i2 = 1;
    std::array<int, 2> constexpr es{i1, i2};

    static_assert(i != all{es});
    static_assert(i1 == all{es});
    static_assert(i != all{std::array<int, 2>{i1, i2}});
    CHECK((i + 1 == all{es}));
  }
}

}  // namespace soro::utls
