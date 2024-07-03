#pragma once

#include "soro/si/units.h"

namespace soro::runtime {

struct train_state {
  static constexpr train_state zero() {
    return train_state{si::time::zero(), si::length::zero(), si::speed::zero()};
  }

  static constexpr train_state invalid() {
    return train_state{si::time::invalid(), si::length::invalid(),
                       si::speed::invalid()};
  }

  train_state operator+(train_state const& other) const {
    train_state result = *this;
    result += other;
    return result;
  }

  train_state& operator+=(train_state const& other) {
    time_ += other.time_;
    dist_ += other.dist_;
    speed_ += other.speed_;
    return *this;
  }

  si::time time_{si::time::zero()};
  si::length dist_{si::length::zero()};
  si::speed speed_{si::speed::zero()};
};

}  // namespace soro::runtime