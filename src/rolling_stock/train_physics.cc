#include "soro/rolling_stock/train_physics.h"

#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/min_element.h"

namespace soro::rs {

using namespace soro::utls;

#if !defined(SERIALIZE)
train_physics::train_physics(soro::vector<traction_vehicle> tvs,
                             si::weight const carriage_weight,
                             si::length const length, si::speed const max_speed,
                             CTC const ctc, FreightTrain const freight)
    : vehicles_{std::move(tvs)},
      carriage_weight_{carriage_weight},
      length_{length},
      max_speed_{max_speed},
      ctc_(ctc),
      freight_(freight) {}
#endif

si::length train_physics::length() const { return length_; }

si::weight train_physics::weight() const { return carriage_weight_; }

si::speed train_physics::max_speed() const {
  auto const vehicle_max_speed =
      utls::min_element(vehicles_, [](auto&& v1, auto&& v2) {
        return v1.max_speed_ < v2.max_speed_;
      })->max_speed_;
  return std::min(vehicle_max_speed, max_speed_);
}

si::force train_physics::tractive_force(si::speed const v) const {
  return accumulate(vehicles_, si::ZERO<si::force>, [&](auto&& acc, auto&& vh) {
    return acc + vh.tractive_curve_(v);
  });
}

si::force train_physics::resistive_force(si::speed const v) const {
  return accumulate(vehicles_, si::ZERO<si::force>, [&](auto&& acc, auto&& vh) {
    return acc + vh.resistance_curve_(v);
  });
}

si::acceleration train_physics::deacceleration() const {
  return vehicles_.front().deacceleration_;
}

CTC train_physics::ctc() const { return this->ctc_; }

FreightTrain train_physics::freight() const { return this->freight_; }

}  // namespace soro::rs
