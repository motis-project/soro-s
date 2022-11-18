#pragma once

#include "soro/rolling_stock/train_series.h"

namespace soro::rs {

struct train_physics {
#if !defined(SERIALIZE)
  train_physics() = default;
  // if we don't serialize we need a constructor since the members are private
  train_physics(soro::vector<traction_vehicle> tvs,
                si::weight const carriage_weight, si::length const length,
                si::speed const max_speed);
#endif

  si::length length() const;

  si::weight weight() const;

  si::speed max_speed() const;

  si::force tractive_force(si::speed const v) const;

  si::force resistive_force(si::speed const v) const;

  si::acceleration deacceleration() const;

#if !defined(SERIALIZE)
  // if we don't need to serialize these members are private
  // since we do not want users to access them directly,
  // as they are subject to change
private:
#endif

  soro::vector<traction_vehicle> vehicles_;
  si::weight carriage_weight_{si::ZERO<si::weight>};
  si::length length_{si::ZERO<si::length>};
  si::speed max_speed_{si::ZERO<si::speed>};
};

}  // namespace soro::rs
