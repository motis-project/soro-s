#pragma once

#include "soro/si/units.h"

namespace soro::runtime::rk4 {

struct get_speed_limit {
  get_speed_limit(si::length const length, si::speed const max_speed,
                  si::speed const target_speed, si::accel const deaccel)
      : length_{length},
        max_speed_{max_speed},
        target_speed_{target_speed},
        deaccel_{deaccel} {}

  bool has_braking_curve() const;
  si::length get_brake_point() const;
  bool is_in_braking_curve(si::length const dist) const;

  // returns the effective speed limit for a point given by dist
  // where:
  // length_ is the total length considered
  // max_speed_ the maximum speed allowed for the total length
  // target_speed_ the speed that is required after length_
  // deaccel_ the deacceleration rate
  si::speed operator()(si::length const dist) const;

  si::length const length_;
  si::speed const max_speed_;
  si::speed const target_speed_;
  si::accel const deaccel_;
};

}  // namespace soro::runtime::rk4
