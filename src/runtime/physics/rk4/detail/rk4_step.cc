#include "soro/runtime/physics/rk4/detail/rk4_step.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/runtime_result.h"

namespace soro::runtime::rk4 {

// result is the delta for a single step with step size h
train_state rk4_step(si::speed const speed, si::time const h,
                     si::slope const slope, rs::train_physics const& tp) {
  train_state result;

  auto const acceleration = [&tp](si::speed const v, si::slope const s) {
    return v > tp.max_speed() ? si::accel::zero() : tp.acceleration(v, s);
  };

  auto const k1_speed = acceleration(speed, slope);

  auto const k2_input = speed + ((h / 2) * k1_speed);
  auto const k2_speed = acceleration(k2_input, slope);

  auto const k3_input = speed + ((h / 2) * k2_speed);
  auto const k3_speed = acceleration(k3_input, slope);

  auto const k4_input = speed + (h * k3_speed);
  auto const k4_speed = acceleration(k4_input, slope);

  auto const k1_dist = speed;
  auto const k2_dist = k2_input;
  auto const k3_dist = k3_input;
  auto const k4_dist = k4_input;

  result.speed_ =
      (h / 6.0) * (k1_speed + (2 * k2_speed) + (2 * k3_speed) + k4_speed);
  result.dist_ =
      (h / 6.0) * (k1_dist + (2 * k2_dist) + (2 * k3_dist) + k4_dist);
  result.time_ = h;

  return result;
}

}  // namespace soro::runtime::rk4