#include "soro/runtime/physics/rk4/accelerate.h"

#include "soro/utls/sassert.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/train_state.h"
#include "soro/runtime/physics/rk4/detail/delta_t.h"
#include "soro/runtime/physics/rk4/detail/get_intersection.h"
#include "soro/runtime/physics/rk4/detail/get_speed_limit.h"
#include "soro/runtime/physics/rk4/detail/rk4_step.h"

namespace soro::runtime::rk4 {

using namespace soro::rs;

// uses a runge kutta 4th order method to numerically solve the
// acceleration differential equation
train_state accelerate4(train_state const init, si::length const stop_at,
                        get_speed_limit const& get_speed_limit,
                        si::slope const slope, si::accel const deaccel,
                        rs::train_physics const& tp) {
  train_state last;
  train_state second_to_last;

  last = init;

  do {  // NOLINT
    second_to_last = last;

    // delta_t is used as the step size h
    last += rk4_step(last.speed_, delta_t, slope, tp);

    // check for:
    // 1. not going over the maximum given distance by stop_at
    // 2. not going backwards (i.e. speed < 0)
    // 3. not going over the given speed limit at the current distance
  } while (last.dist_ < stop_at && last.speed_ > si::speed::zero() &&
           last.speed_ < get_speed_limit(last.dist_));

  utls::sassert(last.time_ > si::time::zero(), "negative time");
  utls::sassert(last.speed_ > si::speed::zero(), "negative speed");

  train_state result;

  auto const quit_due_to_dist = last.dist_ >= stop_at;
  if (quit_due_to_dist) {
    result = detail::get_intersection_at_max_dist(stop_at, second_to_last, last,
                                                  slope, tp);
  }

  auto const quit_due_to_speed =
      !quit_due_to_dist && last.speed_ > get_speed_limit(last.dist_);
  auto const redo_result = result.speed_ > get_speed_limit(stop_at);
  if (quit_due_to_speed || redo_result) {
    result = detail::get_intersection(second_to_last, last, deaccel, slope,
                                      get_speed_limit, tp);

    if (quit_due_to_dist) result.dist_ = stop_at;
    result.speed_ = get_speed_limit(result.dist_);
  }

  utls::sassert(quit_due_to_dist || quit_due_to_speed,
                "quit either due to speed or dist");

  utls::sassert(result.time_ > si::time::zero(), "negative time");
  utls::sassert(result.dist_ > si::length::zero(), "negative dist");
  utls::sassert(result.speed_ > si::speed::zero(), "negative speed");

  utls::sassert(result.dist_ <= stop_at, "too far");
  utls::sassert(result.speed_ <= get_speed_limit(result.dist_), " too fast");

  return result;
}

train_state accelerate_without_braking_curve(
    si::speed const initial_speed, si::length const stop_at,
    get_speed_limit const& get_speed_limit, si::slope const slope,
    si::accel const deaccel, rs::train_physics const& tp) {

  utls::expect(!get_speed_limit.has_braking_curve() ||
                   get_speed_limit.get_brake_point() > stop_at,
               "braking curve calculation required");

  train_state state;
  state.speed_ = initial_speed;
  state.dist_ = si::length::zero();
  state.time_ = si::time::zero();

  auto const result =
      accelerate4(state, stop_at, get_speed_limit, slope, deaccel, tp);

  return result;
}

train_state accelerate_with_braking_curve(
    si::speed const initial_speed, si::length const stop_at,
    get_speed_limit const& get_speed_limit, si::slope const slope,
    si::accel const deaccel, rs::train_physics const& tp) {

  auto const braking_point = get_speed_limit.get_brake_point();

  train_state state;
  state.speed_ = initial_speed;
  state.dist_ = si::length::zero();
  state.time_ = si::time::zero();

  // stop before the braking point
  if (stop_at < braking_point) {
    return accelerate_without_braking_curve(
        initial_speed, stop_at, get_speed_limit, slope, deaccel, tp);
  }

  if (!braking_point.is_zero()) {
    state =
        accelerate4(state, braking_point, get_speed_limit, slope, deaccel, tp);
  }

  // we can accelerate further, even after the braking point
  if (state.dist_ == braking_point &&
      state.speed_ < get_speed_limit(braking_point)) {
    state = accelerate4(state, stop_at, get_speed_limit, slope, deaccel, tp);
  }

  return state;
}

// uses a runge kutta 4th order method to numerically solve the
// acceleration differential equation
train_state accelerate(si::speed const initial_speed, si::speed const max_speed,
                       si::speed const target_speed, si::length max_dist,
                       si::accel const deaccel, si::slope const slope,
                       si::length const stop_at, rs::train_physics const& tp) {
  utls::expect(
      initial_speed < max_speed || tp.tractive_force(initial_speed) <
                                       tp.resistive_force(initial_speed, slope),
      "cannot accelerate");
  utls::expect(deaccel < si::accel::zero(), "positive deaccel");
  utls::expect(max_dist > si::length::zero(), "negative dist");
  utls::expect(stop_at > si::length::zero(), "negative stop");

  get_speed_limit const get_speed_limit{max_dist, max_speed, target_speed,
                                        deaccel};

  auto const result =
      get_speed_limit.has_braking_curve()
          ? accelerate_with_braking_curve(initial_speed, stop_at,
                                          get_speed_limit, slope, deaccel, tp)
          : accelerate_without_braking_curve(
                initial_speed, stop_at, get_speed_limit, slope, deaccel, tp);

  utls::sassert(result.time_ >= si::time::zero(), "negative time");
  utls::sassert(result.speed_ > si::speed::zero(), "negative speed");
  utls::sassert(result.speed_ <= max_speed, "speed exceeds max speed");
  utls::sassert(result.dist_ != max_dist || result.speed_ <= target_speed,
                "speed must be less than target speed at max distance");

  return result;
}

}  // namespace soro::runtime::rk4
