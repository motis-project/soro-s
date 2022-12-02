#pragma once

#include <chrono>

#include "date/date.h"
#include "fmt/format.h"

#include "soro/utls/concepts/is_any_of.h"

namespace soro {

namespace sc = std::chrono;

using i32_seconds = sc::duration<uint32_t, sc::seconds::period>;
using i64_seconds = sc::duration<uint64_t, sc::seconds::period>;
using seconds = i32_seconds;
using minutes = sc::duration<uint32_t, sc::minutes::period>;
using hours = sc::duration<uint32_t, sc::hours::period>;
using days = sc::duration<uint32_t, sc::days::period>;

// an absolute point in time, given as a count of non-leap seconds
// since unix epoch (1970/01/01/ 00:00:00)
// same as sc::time_point<sc::system_clock, sc::seconds>
using absolute_time = date::sys_time<i32_seconds>;

// number of days since 1970/01/01
using anchor_time = date::sys_days;

// relative point in time wrt to an anchor point
// given as a count of non-leap seconds since the anchor point (00:00)
using relative_time = sc::duration<uint32_t, sc::seconds::period>;

using duration2 = i32_seconds;

constexpr absolute_time relative_to_absolute(anchor_time const anchor,
                                             relative_time const relative) {
  return sc::time_point_cast<absolute_time::duration>(anchor) + relative;
}

template <typename T>
concept soro_time =
    utls::is_any_of<T, absolute_time, anchor_time, relative_time, duration2>;

template <soro_time T>
constexpr T const INVALID = T::max();

template <soro_time T>
constexpr bool valid(T const t) {
  return t != INVALID<T>;
}

}  // namespace soro

template <>
struct fmt::formatter<soro::anchor_time> {
  constexpr auto parse(format_parse_context& ctx)  // NOLINT
      -> decltype(ctx.begin()) {
    if (ctx.begin() != ctx.end() && *ctx.begin() != '}') {
      throw format_error("invalid format for anchor time");
    }

    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(soro::anchor_time const at, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", date::year_month_day{at});
  }
};
