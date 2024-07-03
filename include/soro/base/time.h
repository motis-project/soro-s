#pragma once

#include <chrono>
#include <compare>

#include "date/date.h"
#include "fmt/format.h"

#include "soro/utls/concepts/is_any_of.h"
#include "soro/utls/parse_int.h"

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
// same as sc::time_point<sc::system_clock, i32_seconds>
using absolute_time = date::sys_time<i32_seconds>;

// relative point in time wrt to an anchor point
// given as a count of non-leap seconds since an absolute time (the anchor)
using relative_time = i32_seconds;

struct times {
  auto operator<=>(times const&) const = default;

  relative_time arrival_{relative_time::max()};
  relative_time departure_{relative_time::max()};
};

using duration = i32_seconds;

[[nodiscard]] constexpr absolute_time relative_to_absolute(
    absolute_time const anchor, relative_time const relative) {
  return sc::time_point_cast<absolute_time::duration>(anchor) + relative;
}

[[nodiscard]] constexpr relative_time absolute_to_relative(
    absolute_time const anchor, absolute_time const absolute) {
  return absolute - anchor;
}

[[nodiscard]] constexpr absolute_time midnight(absolute_time const t) {
  return sc::floor<days>(t);
}

[[nodiscard]] constexpr absolute_time ymd_to_abs(
    date::year_month_day const ymd) {
  return sc::time_point_cast<absolute_time::duration>(date::sys_days{ymd});
}

[[nodiscard]] constexpr absolute_time rep_to_absolute_time(
    absolute_time::rep const rep) {
  return absolute_time{absolute_time::duration{rep}};
}

[[nodiscard]] constexpr absolute_time::rep absolute_time_to_rep(
    absolute_time const abs) {
  return abs.time_since_epoch().count();
}

[[nodiscard]] inline utls::result<absolute_time> str_to_absolute_time(
    std::string_view const str_time) {
  auto const parsed = utls::try_parse_int<absolute_time::rep>(str_time);

  if (!parsed) return utls::propagate(parsed);

  return rep_to_absolute_time(*parsed);
}

template <typename T>
concept soro_time = utls::is_any_of<T, absolute_time, relative_time, duration>;

template <soro_time T>
constexpr T const INVALID = T::max();

template <soro_time T>
constexpr T const ZERO = T{T::duration::zero()};

template <soro_time T>
constexpr bool valid(T const t) {
  return t != INVALID<T>;
}

inline date::year_month_day parse_date(const char* const c) {
  utls::expect(strlen(c) >= strlen("2022-11-19"),
               "date {} has not the expected minimum length.", c);

  date::year const y{utls::parse_int<int>(c, c + 4)};
  date::month const m{utls::parse_int<unsigned>(c + 5, c + 7)};
  date::day const d{utls::parse_int<unsigned>(c + 8, c + 10)};

  date::year_month_day date{y, m, d};

  utls::ensure(date.ok(), "date {} - {} - {} is not ok!", date.year(),
               date.month(), date.day());

  return date;
};

inline soro::absolute_time parse_dmy_hms(const char* const c) {
  utls::expect(strlen(c) <= strlen("20-02-2024 17:32:30"),
               "datetime {} has not the expected minimum length.", c);

  date::day const day{utls::parse_int<unsigned>(c, c + 2)};
  date::month const month{utls::parse_int<unsigned>(c + 3, c + 5)};
  date::year const year{utls::parse_int<int>(c + 6, c + 10)};

  date::year_month_day const date{year, month, day};

  utls::ensure(date.ok(), "date {} - {} - {} is not ok!", date.year(),
               date.month(), date.day());

  soro::hours h{utls::parse_int<soro::hours::rep>(c + 11, c + 13)};
  soro::minutes m{utls::parse_int<soro::minutes::rep>(c + 14, c + 16)};
  soro::seconds s{utls::parse_int<soro::seconds::rep>(c + 17, c + 19)};

  utls::ensure(h.count() < 24, "hour {} is not valid!", h.count());
  utls::ensure(m.count() < 60, "minute {} is not valid!", m.count());
  utls::ensure(s.count() < 60, "second {} is not valid!", s.count());

  return ymd_to_abs(date) + h + m + s;
}

}  // namespace soro

namespace fmt {

template <>
struct formatter<soro::absolute_time> {
  constexpr auto parse(format_parse_context& ctx)  // NOLINT
      -> decltype(ctx.begin()) {
    if (ctx.begin() != ctx.end() && *ctx.begin() != '}') {
      throw format_error("invalid format for absolute time");
    }

    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(soro::absolute_time const at,
              FormatContext& ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), date::format("%FT%T", at));
  }
};

}  // namespace fmt
