#include "soro/rolling_stock/train_physics.h"

namespace soro::rs {

#if !defined(SERIALIZE)
train_physics::train_physics(traction_vehicle tv,
                             si::weight const carriage_weight,
                             si::length const length, si::speed const max_speed)
    : vehicle_{std::move(tv)},
      carriage_weight_{carriage_weight},
      length_{length},
      max_speed_{max_speed} {}
#endif

si::length train_physics::length() const { return length_; }

si::weight train_physics::weight() const {
  return vehicle_.weight_ + carriage_weight_;
}

si::speed train_physics::max_speed() const {
  return std::min(vehicle_.max_speed_, max_speed_);
}

si::force train_physics::tractive_force(si::speed const v) const {
  return vehicle_.tractive_curve_(v);
}

si::force train_physics::resistive_force(si::speed const v) const {
  return vehicle_.resistance_curve_(v);
}

si::acceleration train_physics::deacceleration() const {
  return vehicle_.deacceleration_;
}

}  // namespace soro::rs
