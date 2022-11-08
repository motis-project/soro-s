#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "soro/rolling_stock/train_physics.h"

namespace soro {

constexpr si::time const DELTA_T = si::from_s(0.5);

struct runtime_result {
  runtime_result() = default;
  runtime_result(si::time const seconds, si::length const meter,
                 si::speed const m_s)
      : time_(seconds), distance_(meter), speed_(m_s) {}

  si::time time_{si::ZERO<si::time>};
  si::length distance_{si::ZERO<si::length>};
  si::speed speed_{si::ZERO<si::speed>};
};

using runtime_results = std::vector<runtime_result>;

inline std::ostream& operator<<(std::ostream& out, runtime_result const& rr) {
  out << "Runtime Result:\n"
      << "Time: " << rr.time_ << " [s]\n"
      << "Distance: " << rr.distance_ << " [m]\n"
      << "Speed: " << rr.speed_ << '\n';
  return out;
}

inline std::ostream& operator<<(std::ostream& out, runtime_results const& rr) {
  for (auto const& entry : rr) {
    out << entry;
  }
  return out;
}

runtime_results accelerate(rs::train_physics const& tp, si::speed initial_speed,
                           si::speed target_speed, si::length max_distance);

runtime_results brake(rs::train_physics const& p, si::speed initial_speed,
                      si::speed target_speed);

runtime_results coast(si::speed current_speed, si::length distance);

}  // namespace soro