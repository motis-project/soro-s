#include "soro/runtime/physics/rk4/detail/get_intersection.h"

#include <utility>

#include "soro/base/fp_precision.h"

#include "soro/utls/sassert.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/train_state.h"
#include "soro/runtime/physics/rk4/detail/get_speed_limit.h"
#include "soro/runtime/physics/rk4/detail/interpolate_intersection.h"
#include "soro/runtime/physics/rk4/detail/newton_interpolator.h"
#include "soro/runtime/physics/rk4/detail/polynom.h"
#include "soro/runtime/physics/rk4/detail/sampling_points.h"

namespace soro::runtime::rk4::detail {

using namespace soro::rs;

// used as a unit for the derivative of the speed limit function
using speed_prime =
    decltype(std::declval<si::unitless>() / std::declval<si::time>());

constexpr auto epsilon_v = si::speed{FP_PRECISION<si::speed::precision>};

// uses a linear interpolation to find the intersection of the speed and max
si::length get_linear_dist(train_state const& initial,
                           train_state const& current,
                           get_speed_limit const& get_speed_limit) {
  auto const alpha =
      (current.speed_ - initial.speed_) / (current.dist_ - initial.dist_);
  auto const beta =
      (get_speed_limit(current.dist_) - get_speed_limit(initial.dist_)) /
      (current.dist_ - initial.dist_);

  auto const result =
      ((initial.speed_ - get_speed_limit(initial.dist_)) / (beta - alpha)) +
      initial.dist_;

  utls::ensure(result >= initial.dist_ && result <= current.dist_,
               "start value must be in range");

  return result;
}

si::time get_linear_time(train_state const& initial, train_state const& current,
                         si::length const dist) {
  return initial.time_ + (current.time_ - initial.time_) /
                             (current.dist_ - initial.dist_) *
                             (dist - initial.dist_);
}

train_state get_intersection(train_state const second_to_last,
                             train_state const last, si::accel const deaccel,
                             si::slope const slope,
                             get_speed_limit const& get_speed_limit,
                             train_physics const& tp) {
  // second_to_last and last are the last two results of the rk4 method
  utls::expect(second_to_last.dist_ < last.dist_,
               "initial dist must be less than current dist");
  utls::expect(second_to_last.time_ < last.time_,
               "initial time must be less than current time");

  train_state result;

  sampling_points<poly_deg> const sps(second_to_last, last, slope, tp);

  newton_interpolator<poly_deg, si::speed, speed_prime> const speed(sps);
  newton_interpolator<poly_deg, si::time, si::time> const time(sps);

  auto const start_value =
      get_linear_dist(second_to_last, last, get_speed_limit);

  auto const max_speed = [&](si::length const dist) {
    utls::expect(dist >= si::length::zero());

    auto const max = get_speed_limit(dist);

    auto const before_braking = !get_speed_limit.is_in_braking_curve(dist);
    auto const max_speed_prime =
        before_braking ? speed_prime{0.0} : deaccel / max;

    return std::pair<si::speed, speed_prime>{max, max_speed_prime};
  };

  // speed and max_speed objects are able to evaluate the current speed and max
  // speed for a given distance. additionally, they evaluate the first
  // derivative of the current speed and max speed function.
  result.dist_ =
      interpolate_intersection(start_value, speed, max_speed, epsilon_v);
  result.time_ = time.interpolate(result.dist_);
  result.speed_ = speed.interpolate(result.dist_);

  if (result.dist_ < second_to_last.dist_ || result.dist_ > last.dist_) {
    // TODO(julian) bit of a hack, if the newton interpolation does not
    // land in the given range we simply take the linear interpolation
    result.dist_ = start_value;
  }

  if (result.time_ < second_to_last.time_ || result.time_ > last.time_) {
    // TODO(julian) bit of a hack, if the newton interpolation does not
    // land in the given range we simply take the linear interpolation
    result.time_ = get_linear_time(second_to_last, last, result.dist_);
  }

  utls::sassert(result.time_ > si::time::zero(), "negative time");
  utls::sassert(result.dist_ > si::length::zero(), "negative dist");
  utls::sassert(result.speed_ > si::speed::zero(), "negative speed");

  utls::sassert(
      second_to_last.time_ <= result.time_ && result.time_ <= last.time_,
      "time out of range");
  utls::sassert(
      second_to_last.dist_ <= result.dist_ && result.dist_ <= last.dist_,
      "distance out of range");

  return result;
}

train_state get_intersection_at_max_dist(si::length const length,
                                         train_state const second_to_last,
                                         train_state const last,
                                         si::slope const slope,
                                         train_physics const& tp) {
  sampling_points<poly_deg> const sps(second_to_last, last, slope, tp);

  newton_interpolator<poly_deg, si::speed, speed_prime> const speed(sps);
  newton_interpolator<poly_deg, si::time, si::time> const time(sps);

  train_state result;
  result.dist_ = length;
  result.speed_ = speed.interpolate(result.dist_);
  result.time_ = time.interpolate(result.dist_);

  if (result.time_ < second_to_last.time_ || result.time_ > last.time_) {
    // TODO(julian) bit of a hack, if the newton interpolation does not
    // land in the given range we simply take the linear interpolation
    result.time_ = get_linear_time(second_to_last, last, result.dist_);
  }

  utls::sassert(
      second_to_last.time_ <= result.time_ && result.time_ <= last.time_,
      "time out of range");
  utls::sassert(
      second_to_last.dist_ <= result.dist_ && result.dist_ <= last.dist_,
      "distance out of range");

  return result;
}

}  // namespace soro::runtime::rk4::detail