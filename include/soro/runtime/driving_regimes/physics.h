#pragma once

#include <tuple>

#include "soro/runtime/driving_regimes/general.h"

namespace soro::runtime {

/**
 * Calculates the needed time for the interval (start -> end) for an arbitrary
 * object with a current velocity vel0 at start and a constant acceleration in
 * the interval. Additionally the velocity of the object at end, therefore after
 * accelerating and passing the interval is calculated.
 *
 * @param acc acceleration in the interval (start -> end), a number in R
 * @param vel0 velocity at start, must be >= 0
 * @param start start of the interval in m, must be >=0
 * @param end end of the interval in m, must be > 0
 * @return a tuple: time needed to cross interval, vel1 at end
 */
std::tuple<si::time, si::speed> accelerate(si::acceleration acc, si::speed vel0,
                                           si::length start, si::length end);

/**
 * Calculates the time needed for the interval (start -> end) for an arbitrary
 * object with a velocity vel1 at end and a constant acceleration in the
 * interval. Additionally the velocity of the object at start, therefore before
 * accelerating and passing the interval is calculated.
 *
 * @param acc acceleration in the interval (start -> end), must be < 0
 * @param vel1 velocity at end, must be >= 0
 * @param start start of the interval in m, must be >= 0
 * @param end end of the interval in m, must be > 0
 * @return a tuple: time needed to pass interval, vel0 at start
 */
std::tuple<si::time, si::speed> accelerate_reverse(si::acceleration acc,
                                                   si::speed vel1,
                                                   si::length start,
                                                   si::length end);

}  // namespace soro::runtime
