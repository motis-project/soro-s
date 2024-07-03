#include "soro/runtime/physics/rk4/detail/get_speed_limit.h"

#include <algorithm>

#include "soro/utls/sassert.h"

#include "soro/si/units.h"

#include "soro/runtime/physics/rk4/brake.h"

namespace soro::runtime::rk4 {

bool get_speed_limit::has_braking_curve() const {
  return max_speed_ > target_speed_;
}

si::length get_speed_limit::get_brake_point() const {
  auto const brake_result = brake(max_speed_, target_speed_, deaccel_);
  auto const result = length_ - brake_result.dist_;
  return result.smooth();
}

bool get_speed_limit::is_in_braking_curve(si::length const dist) const {
  //  utls::expect(dist >= si::length::zero(), "negative distance");
  //  utls::expect(dist <= length_, "distance exceeds interval length");

  // we don't even have a braking curve here ...
  if (!has_braking_curve()) return false;

  if (dist == length_) return true;

  return dist >= get_brake_point();
}

si::speed get_speed_limit::operator()(si::length const dist) const {
  //  utls::expect(dist >= si::length::zero(), "negative distance");
  //  utls::expect(dist <= length_, "distance exceeds interval length");

  if (dist == length_) return std::min(target_speed_, max_speed_);

  auto const before_braking = !is_in_braking_curve(dist);

  // if the point given by dist is located before the braking point,
  // we can return the general effective speed limit
  if (before_braking) return max_speed_;

  auto const t1 = target_speed_.pow<2>();
  auto const t2 = 2 * deaccel_ * (length_ - dist);

  utls::sassert(t1 > t2, "negative time in sqrt");

  auto const result = (t1 - t2).sqrt();

  return result;
}

}  // namespace soro::runtime::rk4
