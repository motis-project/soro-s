#include "soro/runtime/runtime_physics.h"

#include <cmath>
#include <sstream>

#include "soro/si/constants.h"
#include "utl/verify.h"

namespace soro {

using namespace soro::si;

runtime_results coast(speed const current_speed, length const distance) {
  auto const total_time = distance / current_speed;

  runtime_results coasting_results;
  coasting_results.reserve(
      static_cast<size_t>(as_precision(total_time / DELTA_T)));

  time current_time = ZERO<time>;
  length current_distance = ZERO<length>;
  coasting_results.emplace_back(current_time, current_distance, current_speed);

  while (current_distance < distance) {
    current_time += DELTA_T;
    current_distance += current_speed * DELTA_T;
    coasting_results.emplace_back(current_time, current_distance,
                                  current_speed);
  }

  if (coasting_results.size() > 1) {
    coasting_results.back().distance_ = distance;
  }

  return coasting_results;
}

runtime_results brake(rs::train_physics const& tp, speed const initial_speed,
                      speed const target_speed) {
  utl::verify(initial_speed > target_speed,
              "Target speed higher than current speed in deacceleration!");

  speed const delta_speed = initial_speed - target_speed;
  time const braking_time = -delta_speed / tp.deacceleration();

  runtime_results deaccel_results;
  deaccel_results.reserve(
      static_cast<size_t>(as_s(braking_time) / as_s(DELTA_T)));

  length const braking_distance =
      (0.5 * tp.deacceleration() * pow<2>(braking_time)) +
      (initial_speed * braking_time);

  time current_time = ZERO<time>;
  length current_distance = ZERO<length>;
  speed current_speed = initial_speed;
  deaccel_results.emplace_back(current_time, current_distance, current_speed);

  while (current_time < braking_time) {
    current_time += DELTA_T;
    current_distance += current_speed * DELTA_T;
    current_speed += tp.deacceleration() * DELTA_T;

    deaccel_results.emplace_back(current_time, current_distance, current_speed);
  }

  if (deaccel_results.size() > 1) {
    deaccel_results.back().distance_ = braking_distance;
    deaccel_results.back().speed_ = target_speed;
  }

  return deaccel_results;
}

runtime_results accelerate(rs::train_physics const& tp,
                           speed const initial_speed, speed const target_speed,
                           length const max_distance) {
  runtime_results rr;

  time current_time = ZERO<time>;
  length current_distance = ZERO<length>;
  speed current_speed = initial_speed;

  rr.emplace_back(current_time, current_distance, current_speed);

  while (current_speed < tp.max_speed() && current_speed < target_speed &&
         current_distance < max_distance) {

    force const tractive = tp.tractive_force(current_speed);
    force const resistive = tp.resistive_force(current_speed);

    acceleration const acceleration =
        (tractive - resistive) / (tp.weight() * MASS_FACTOR);

    speed const prev_speed = current_speed;
    current_speed += acceleration * DELTA_T;
    current_distance += (prev_speed + current_speed) * 0.5 * DELTA_T;
    current_time += DELTA_T;

    rr.emplace_back(current_time, current_distance, current_speed);
  }

  if (rr.back().distance_ > max_distance) {
    rr.pop_back();

    if (rr.size() > 1) {
      rr.back().distance_ = max_distance;
    }
  }

  return rr;
}

}  // namespace soro