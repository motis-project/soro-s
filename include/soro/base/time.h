#pragma once

#include <chrono>

#include "date/date.h"

namespace soro {

namespace sc = std::chrono;

using duration_t = uint32_t;

using seconds = std::chrono::duration<duration_t, std::chrono::seconds::period>;
using minutes = std::chrono::duration<duration_t, std::chrono::minutes::period>;
using hours = std::chrono::duration<duration_t, std::chrono::hours::period>;
using days = sc::duration<duration_t, sc::days::period>;

// an absolute point in time, given as a count of non-leap seconds
// since unix epoch (1970/01/01/ 00:00:00)
// same as sc::time_point<sc::system_clock, sc::seconds>
using absolute_time = date::sys_seconds;

static constexpr auto const INVALID_ABSOLUTE_TIME = absolute_time::max();

// relative point in time wrt an arbitrary point in time (the anchor point),
// given as a count of non-leap seconds since the anchor point
using relative_time = seconds;
static constexpr auto const INVALID_RELATIVE_TIME = relative_time::max();

constexpr auto valid(relative_time const rt) {
  return rt != INVALID_RELATIVE_TIME;
}

using duration2 = seconds;
static constexpr auto const INVALID_DURATION = duration2::max();

constexpr absolute_time relative_to_absolute(date::year_month_day const anchor,
                                             relative_time const relative) {
  return sc::time_point_cast<sc::seconds>(static_cast<date::sys_days>(anchor)) +
         relative;
}

static constexpr auto const seconds_in_a_day =
    sc::duration_cast<seconds>(days{1});

}  // namespace soro