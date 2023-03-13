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

// relative point in time wrt to an anchor point
// given as a count of non-leap seconds since an absolute time (the anchor)
using relative_time = i32_seconds;

using duration2 = i32_seconds;

constexpr absolute_time relative_to_absolute(absolute_time const anchor,
                                             relative_time const relative) {
  return sc::time_point_cast<absolute_time::duration>(anchor) + relative;
}

constexpr relative_time absolute_to_relative(absolute_time const anchor,
                                             absolute_time const absolute) {
  return absolute - anchor;
}

constexpr absolute_time midnight(absolute_time const t) {
  return sc::floor<days>(t);
}

constexpr absolute_time ymd_to_abs(date::year_month_day const ymd) {
  return sc::time_point_cast<absolute_time::duration>(date::sys_days{ymd});
}

template <typename T>
concept soro_time = utls::is_any_of<T, absolute_time, relative_time, duration2>;

template <soro_time T>
constexpr T const INVALID = T::max();

template <soro_time T>
constexpr bool valid(T const t) {
  return t != INVALID<T>;
}

}  // namespace soro

template <>
struct fmt::formatter<soro::absolute_time> {
  constexpr auto parse(format_parse_context& ctx)  // NOLINT
      -> decltype(ctx.begin()) {
    if (ctx.begin() != ctx.end() && *ctx.begin() != '}') {
      throw format_error("invalid format for absolute time");
    }

    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(soro::absolute_time const at, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), date::format("%FT%T", at));
  }
};
