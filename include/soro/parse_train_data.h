#pragma once

#include <array>
#include <iosfwd>
#include <string>
#include <vector>

namespace soro {

using meter_per_second = float;

struct tractive_force {
  friend std::ostream& operator<<(std::ostream& out, tractive_force const& f);
  bool operator<(tractive_force const& o) const;
  meter_per_second from_, to_;
  std::array<float, 3> tractive_force_;
  std::array<float, 3> coefficients_;
};

struct train_physics {
  friend std::ostream& operator<<(std::ostream& out, train_physics const& tp);
  std::string name_;
  float weight_t_;
  meter_per_second max_speed_;
  std::array<float, 3> running_resistance_;
  std::vector<tractive_force> tractive_force_;
};

std::vector<train_physics> parse_train_data(std::string const&);

}  // namespace soro