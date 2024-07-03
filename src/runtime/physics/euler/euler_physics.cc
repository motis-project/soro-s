#include "soro/runtime/physics/euler/euler_physics.h"

#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/runtime_result.h"

namespace soro::runtime::euler {

using namespace soro::si;

constexpr si::time const DELTA_T = si::from_s(0.1);

runtime_results brake(speed const initial_speed, speed const target_speed,
                      accel const deacceleration) {
  utl::verify(initial_speed > target_speed,
              "Target speed higher than current speed in deacceleration!");
  utl::verify(deacceleration < accel::zero(),
              "negative deacceleration required, got {}", deacceleration);

  speed const delta_speed = initial_speed - target_speed;
  time const braking_time = -delta_speed / deacceleration;

  runtime_results deaccel_results;
  deaccel_results.reserve(
      static_cast<size_t>(as_s(braking_time) / as_s(DELTA_T)));

  length const braking_distance =
      (0.5 * deacceleration * braking_time.pow<2>()) +
      (initial_speed * braking_time);

  auto current_time = time::zero();
  auto current_distance = length::zero();
  auto current_speed = initial_speed;
  deaccel_results.emplace_back(current_time, current_distance, current_speed);

  while (current_time < braking_time) {
    current_time += DELTA_T;
    current_distance += current_speed * DELTA_T;
    current_speed += deacceleration * DELTA_T;

    deaccel_results.emplace_back(current_time, current_distance, current_speed);
  }

  if (deaccel_results.size() > 1) {
    deaccel_results.back().dist_ = braking_distance;
    deaccel_results.back().speed_ = target_speed;
  }

  return deaccel_results;
}

runtime_results accelerate(rs::train_physics const& tp,
                           speed const initial_speed, speed const target_speed,
                           length const max_distance, slope const slope) {
  runtime_results rr;

  auto current_time = time::zero();
  auto current_distance = length::zero();
  auto current_speed = initial_speed;

  rr.emplace_back(current_time, current_distance, current_speed);

  while (current_speed < tp.max_speed(target_speed) &&
         current_speed < target_speed && current_distance < max_distance) {

    auto const acceleration = tp.acceleration(current_speed, slope);

    auto const prev_speed = current_speed;
    current_speed += acceleration * DELTA_T;
    current_distance += (prev_speed + current_speed) * 0.5 * DELTA_T;
    current_time += DELTA_T;

    //    utls::sassert(acceleration > acceleration::zero(), "slowing down");
    utl::verify(current_speed > speed::zero(), "going backwards");

    rr.emplace_back(current_time, current_distance, current_speed);
  }

  if (rr.size() > 1 && rr.back().dist_ > max_distance) {
    rr.back().dist_ = max_distance;
  }

  if (rr.size() > 1 && rr.back().speed_ > target_speed) {
    rr.back().speed_ = target_speed;
  }

  return rr;
}

runtime_results cruise(speed const current_speed, length const distance) {
  utls::sassert(!current_speed.is_zero(), "got zero speed for coasting");

  auto const total_time = distance / current_speed;

  runtime_results coasting_results;
  coasting_results.reserve(
      static_cast<size_t>(as_precision(total_time / DELTA_T)));

  auto current_time = time::zero();
  auto current_distance = length::zero();
  coasting_results.emplace_back(current_time, current_distance, current_speed);

  while (current_distance < distance && current_distance != distance) {
    current_time += DELTA_T;
    current_distance += current_speed * DELTA_T;
    coasting_results.emplace_back(current_time, current_distance,
                                  current_speed);
  }

  if (coasting_results.size() > 1) {
    coasting_results.back().dist_ = distance;
  }

  return coasting_results;
}

}  // namespace soro::runtime::euler