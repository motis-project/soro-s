#include "doctest/doctest.h"

#include "soro/utls/narrow.h"

namespace soro::utls::detail {

// unsigned x unsigned

static_assert(!is_narrowing<uint8_t, uint8_t>());
static_assert(is_narrowing<uint8_t, uint16_t>());
static_assert(is_narrowing<uint8_t, uint32_t>());
static_assert(is_narrowing<uint8_t, uint64_t>());

static_assert(!is_narrowing<uint16_t, uint8_t>());
static_assert(!is_narrowing<uint32_t, uint8_t>());
static_assert(!is_narrowing<uint64_t, uint8_t>());

static_assert(!is_narrowing<uint16_t, uint16_t>());
static_assert(is_narrowing<uint16_t, uint32_t>());
static_assert(is_narrowing<uint16_t, uint64_t>());

static_assert(!is_narrowing<uint32_t, uint16_t>());
static_assert(!is_narrowing<uint64_t, uint16_t>());

static_assert(!is_narrowing<uint32_t, uint32_t>());
static_assert(is_narrowing<uint32_t, uint64_t>());

static_assert(!is_narrowing<uint64_t, uint32_t>());

static_assert(!is_narrowing<uint64_t, uint64_t>());

// signed x signed

static_assert(!is_narrowing<int8_t, int8_t>());
static_assert(is_narrowing<int8_t, int16_t>());
static_assert(is_narrowing<int8_t, int32_t>());
static_assert(is_narrowing<int8_t, int64_t>());

static_assert(!is_narrowing<int16_t, int8_t>());
static_assert(!is_narrowing<int32_t, int8_t>());
static_assert(!is_narrowing<int64_t, int8_t>());

static_assert(!is_narrowing<int16_t, int16_t>());
static_assert(is_narrowing<int16_t, int32_t>());
static_assert(is_narrowing<int16_t, int64_t>());

static_assert(!is_narrowing<int32_t, int16_t>());
static_assert(!is_narrowing<int64_t, int16_t>());

static_assert(!is_narrowing<int32_t, int32_t>());
static_assert(is_narrowing<int32_t, int64_t>());

static_assert(!is_narrowing<int64_t, int32_t>());

static_assert(!is_narrowing<int64_t, int64_t>());

// signed x unsigned

static_assert(is_narrowing<int8_t, uint8_t>());
static_assert(!is_narrowing<int16_t, uint8_t>());
static_assert(!is_narrowing<int32_t, uint8_t>());
static_assert(!is_narrowing<int64_t, uint8_t>());

static_assert(is_narrowing<int8_t, uint16_t>());
static_assert(is_narrowing<int16_t, uint16_t>());
static_assert(!is_narrowing<int32_t, uint16_t>());
static_assert(!is_narrowing<int64_t, uint16_t>());

static_assert(is_narrowing<int8_t, uint32_t>());
static_assert(is_narrowing<int16_t, uint32_t>());
static_assert(is_narrowing<int32_t, uint32_t>());
static_assert(!is_narrowing<int64_t, uint32_t>());

static_assert(is_narrowing<int8_t, uint64_t>());
static_assert(is_narrowing<int16_t, uint64_t>());
static_assert(is_narrowing<int32_t, uint64_t>());
static_assert(is_narrowing<int64_t, uint64_t>());

// unsigned x signed

static_assert(is_narrowing<uint8_t, int8_t>());
static_assert(is_narrowing<uint8_t, int16_t>());
static_assert(is_narrowing<uint8_t, int32_t>());
static_assert(is_narrowing<uint8_t, int64_t>());

static_assert(is_narrowing<uint16_t, int8_t>());
static_assert(is_narrowing<uint16_t, int16_t>());
static_assert(is_narrowing<uint16_t, int32_t>());
static_assert(is_narrowing<uint16_t, int64_t>());

static_assert(is_narrowing<uint32_t, int8_t>());
static_assert(is_narrowing<uint32_t, int16_t>());
static_assert(is_narrowing<uint32_t, int32_t>());
static_assert(is_narrowing<uint32_t, int64_t>());

static_assert(is_narrowing<uint64_t, int8_t>());
static_assert(is_narrowing<uint64_t, int16_t>());
static_assert(is_narrowing<uint64_t, int32_t>());
static_assert(is_narrowing<uint64_t, int64_t>());

TEST_SUITE("fits") {
  TEST_CASE("fits uint32_t") {
    CHECK(fits<uint32_t>(std::numeric_limits<uint8_t>::max()));
    CHECK(fits<uint32_t>(std::numeric_limits<uint16_t>::max()));
    CHECK(fits<uint32_t>(std::numeric_limits<uint32_t>::max()));
    CHECK(!fits<uint32_t>(std::numeric_limits<uint64_t>::max()));

    CHECK(fits<uint32_t>(std::numeric_limits<uint8_t>::min()));
    CHECK(fits<uint32_t>(std::numeric_limits<uint16_t>::min()));
    CHECK(fits<uint32_t>(std::numeric_limits<uint32_t>::min()));
    CHECK(fits<uint32_t>(std::numeric_limits<uint64_t>::min()));

    CHECK(fits<uint32_t>(std::numeric_limits<int8_t>::max()));
    CHECK(fits<uint32_t>(std::numeric_limits<int16_t>::max()));
    CHECK(fits<uint32_t>(std::numeric_limits<int32_t>::max()));
    CHECK(!fits<uint32_t>(std::numeric_limits<int64_t>::max()));

    CHECK(!fits<uint32_t>(std::numeric_limits<int8_t>::min()));
    CHECK(!fits<uint32_t>(std::numeric_limits<int16_t>::min()));
    CHECK(!fits<uint32_t>(std::numeric_limits<int32_t>::min()));
    CHECK(!fits<uint32_t>(std::numeric_limits<int64_t>::min()));
  }

  TEST_CASE("fits int32_t") {
    CHECK(fits<int32_t>(std::numeric_limits<uint8_t>::max()));
    CHECK(fits<int32_t>(std::numeric_limits<uint16_t>::max()));
    CHECK(!fits<int32_t>(std::numeric_limits<uint32_t>::max()));
    CHECK(!fits<int32_t>(std::numeric_limits<uint64_t>::max()));

    CHECK(fits<int32_t>(std::numeric_limits<uint8_t>::min()));
    CHECK(fits<int32_t>(std::numeric_limits<uint16_t>::min()));
    CHECK(fits<int32_t>(std::numeric_limits<uint32_t>::min()));
    CHECK(fits<int32_t>(std::numeric_limits<uint64_t>::min()));

    CHECK(fits<int32_t>(std::numeric_limits<int8_t>::max()));
    CHECK(fits<int32_t>(std::numeric_limits<int16_t>::max()));
    CHECK(fits<int32_t>(std::numeric_limits<int32_t>::max()));
    CHECK(!fits<int32_t>(std::numeric_limits<int64_t>::max()));

    CHECK(fits<int32_t>(std::numeric_limits<int8_t>::min()));
    CHECK(fits<int32_t>(std::numeric_limits<int16_t>::min()));
    CHECK(fits<int32_t>(std::numeric_limits<int32_t>::min()));
    CHECK(!fits<int32_t>(std::numeric_limits<int64_t>::min()));
  }
}

}  // namespace soro::utls::detail
