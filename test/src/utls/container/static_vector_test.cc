#include "doctest/doctest.h"

#include <cstdint>
#include <iterator>
#include <limits>
#include <utility>

#include "utl/erase_if.h"

#include "soro/base/soro_types.h"

#include "soro/utls/container/static_vector.h"

using namespace soro::utls;

struct tester {
  tester() = default;
  tester(uint32_t val, uint32_t* destroy, uint32_t* move, uint32_t* copy)
      : val_{val},
        destroy_{destroy},
        move_{move},
        copy_{copy},
        initialized_{true} {}

  tester(tester&& other) noexcept {
    copy_ptrs(std::forward<tester>(other));
    ++(*move_);
  }

  tester& operator=(tester&& other) noexcept {
    if (this == &other) return *this;

    copy_ptrs(std::forward<tester>(other));
    ++(*move_);
    return *this;
  }

  tester(tester const& other) {
    copy_ptrs(other);
    ++(*copy_);
  }

  tester& operator=(tester const& other) {
    if (this == &other) return *this;

    copy_ptrs(other);
    ++(*copy_);
    return *this;
  }

  ~tester() {
    if (initialized_) ++(*destroy_);
  }

  bool operator==(tester const& other) const = default;

  void copy_ptrs(tester const& other) {
    val_ = other.val_;
    destroy_ = other.destroy_;
    move_ = other.move_;
    copy_ = other.copy_;
  }

  uint32_t val_{std::numeric_limits<uint32_t>::max()};
  uint32_t* destroy_{nullptr};
  uint32_t* move_{nullptr};
  uint32_t* copy_{nullptr};
  bool initialized_{false};
};

auto const values = [](auto&& v) {
  return soro::to_vec(v, [](auto&& t) { return t.val_; });
};

TEST_SUITE("static vector") {
  TEST_CASE("static vector 1") {
    static_vector<uint32_t, 10> v;
    for (auto i = 0U; i < 10; ++i) {
      v.emplace_back(i);
    }

    CHECK_EQ(v.size(), 10);
    for (auto i = 0U; i < 10; ++i) {
      CHECK_EQ(v[i], i);
    }
  }

  TEST_CASE("erase") {
    uint32_t destroyed = 0;
    uint32_t moved = 0;
    uint32_t copied = 0;

    static_vector<tester, 10> v;
    for (auto i = 0U; i < v.capacity(); ++i) {
      v.emplace_back(i, &destroyed, &moved, &copied);
    }

    SUBCASE("front") {
      v.erase(std::begin(v));
      CHECK_EQ(destroyed, 1);
      CHECK_EQ(moved, 9);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 9);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{1, 2, 3, 4, 5, 6, 7, 8, 9};
      CHECK_EQ(vals, expected);
    }

    SUBCASE("erase middle 1") {
      v.erase(std::next(std::begin(v), 4));
      CHECK_EQ(destroyed, 1);
      CHECK_EQ(moved, 5);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 9);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{0, 1, 2, 3, 5, 6, 7, 8, 9};
      CHECK_EQ(vals, expected);
    }

    SUBCASE("erase middle 2") {
      v.erase(std::next(std::begin(v), 5));
      CHECK_EQ(destroyed, 1);
      CHECK_EQ(moved, 4);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 9);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{0, 1, 2, 3, 4, 6, 7, 8, 9};
      CHECK_EQ(vals, expected);
    }

    SUBCASE("erase back") {
      v.erase(std::prev(std::end(v)));
      CHECK_EQ(destroyed, 1);
      CHECK_EQ(moved, 0);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 9);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{0, 1, 2, 3, 4, 5, 6, 7, 8};
      CHECK_EQ(vals, expected);
    }
  }

  TEST_CASE("erase range") {
    uint32_t destroyed = 0;
    uint32_t moved = 0;
    uint32_t copied = 0;

    static_vector<tester, 10> v;
    for (auto i = 0U; i < v.capacity(); ++i) {
      v.emplace_back(i, &destroyed, &moved, &copied);
    }

    SUBCASE("front") {
      v.erase(std::begin(v), std::next(std::begin(v), 3));
      CHECK_EQ(destroyed, 3);
      CHECK_EQ(moved, 7);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 7);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{3, 4, 5, 6, 7, 8, 9};
      CHECK_EQ(vals, expected);
    }

    SUBCASE("erase middle 1") {
      v.erase(std::begin(v) + 4, std::begin(v) + 8);
      CHECK_EQ(destroyed, 4);
      CHECK_EQ(moved, 2);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 6);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{0, 1, 2, 3, 8, 9};
      CHECK_EQ(vals, expected);
    }

    SUBCASE("erase middle 2") {
      v.erase(std::begin(v) + 5, std::begin(v) + 7);
      CHECK_EQ(destroyed, 2);
      CHECK_EQ(moved, 3);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 8);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{0, 1, 2, 3, 4, 7, 8, 9};
      CHECK_EQ(vals, expected);
    }

    SUBCASE("erase back") {
      v.erase(std::begin(v) + 7, std::end(v));
      CHECK_EQ(destroyed, 3);
      CHECK_EQ(moved, 0);
      CHECK_EQ(copied, 0);
      CHECK_EQ(v.size(), 7);
      auto const vals = values(v);
      auto const expected = soro::vector<uint32_t>{0, 1, 2, 3, 4, 5, 6};
      CHECK_EQ(vals, expected);
    }
  }

  TEST_CASE("erase if") {
    static_vector<uint32_t, 10> v;
    for (auto i = 0U; i < 10; ++i) {
      v.emplace_back(i);
    }

    utl::erase_if(v, [](auto&& i) { return i % 2 == 0; });

    static_vector<uint32_t, 10> const expected = {1, 3, 5, 7, 9};

    CHECK_EQ(v, expected);
  }
}
