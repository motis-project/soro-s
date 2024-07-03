#include "doctest/doctest.h"

#include <cstddef>
#include <limits>

#include "soro/utls/container/optional.h"

using namespace soro;
using namespace soro::utls;

TEST_SUITE("optional") {

  TEST_CASE("optional simple") {
    using t = std::size_t;
    optional<t> opt;

    CHECK(!opt.has_value());

    opt = optional{t{400}};

    CHECK(opt.has_value());
    CHECK_EQ(*opt, t{400});

    if (opt) {
      CHECK(opt.has_value());
    } else {
      CHECK(!opt.has_value());
    }

    CHECK_NE(opt.value(), std::numeric_limits<t>::max());
  }

  TEST_CASE("make optional") {
    using t = std::size_t;

    auto opt = make_optional<t>(400);

    CHECK(opt.has_value());
    CHECK_NE(opt.value(), std::numeric_limits<t>::max());

    opt = optional{t{401}};

    CHECK(opt.has_value());
    CHECK_EQ(*opt, t{401});
  }

  TEST_CASE("optional value_or") {
    using t = std::size_t;

    auto const opt = make_optional<t>(400);
    optional<t> const empty;

    auto const result1 = opt.value_or(42);
    auto const result2 = empty.value_or(42);

    CHECK_EQ(result1, 400);
    CHECK_EQ(result2, 42);
  }

  TEST_CASE("optional or_else") {
    using t = std::size_t;

    auto const opt = make_optional<t>(400);
    optional<t> const empty;

    auto const result1 = opt.or_else([]() { return optional<t>{42}; });
    auto const result2 = empty.or_else([]() { return optional<t>{42}; });

    CHECK(result1.has_value());
    CHECK_EQ(*result1, 400);

    CHECK(result2.has_value());
    CHECK_EQ(*result2, 42);
  }

  TEST_CASE("optional transform") {
    using t = std::size_t;

    auto const opt = make_optional<t>(400);
    optional<t> const empty;

    auto const result1 =
        opt.transform([](auto&& o) { return static_cast<int>(o); });
    auto const result2 =
        empty.transform([](auto&& o) { return static_cast<int>(o); });

    CHECK(result1.has_value());
    CHECK_EQ(*result1, int{400});

    CHECK(!result2.has_value());
  }

  TEST_CASE("optional and_then") {
    using t = std::size_t;

    auto const opt = make_optional<t>(400);
    optional<t> const empty;

    auto const result1 = opt.and_then(
        [](auto&& o) { return optional<int>{static_cast<int>(o)}; });
    auto const result2 = empty.and_then(
        [](auto&& o) { return optional<int>{static_cast<int>(o)}; });

    CHECK(result1.has_value());
    CHECK_EQ(*result1, int{400});

    CHECK(!result2.has_value());
  }

  TEST_CASE("optional emplace") {
    using t = std::size_t;

    auto opt = make_optional<t>(400);
    optional<t> empty;

    CHECK(opt.has_value());
    CHECK_EQ(*opt, 400);

    CHECK(!empty.has_value());

    opt.emplace(t{420});
    empty.emplace(t{420});

    CHECK(opt.has_value());
    CHECK(empty.has_value());

    CHECK_EQ(opt, empty);
    CHECK_EQ(*opt, *empty);
  }
}
