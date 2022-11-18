#pragma once

#include <chrono>

#include "date/date.h"

namespace soro {

namespace sc = std::chrono;

using relative_time_t = uint32_t;
using duration_t = uint32_t;

// an absolute point in time, given as a count of non-leap seconds
// since unix epoch (1970/01/01/ 00:00:00)
// same as sc::time_point<sc::system_clock, sc::seconds>
using absolute_time = date::sys_seconds;

// relative point in time wrt an arbitrary point in time (the anchor point),
// given as a count of non-leap seconds since the anchor point
using relative_time = sc::duration<relative_time_t, sc::seconds::period>;

using duration2 = sc::duration<duration_t, sc::seconds::period>;

constexpr absolute_time relative_to_absolute(date::year_month_day const anchor,
                                             relative_time const relative) {
  return sc::time_point_cast<sc::seconds>(static_cast<date::sys_days>(anchor)) +
         relative;
}

}  // namespace soro