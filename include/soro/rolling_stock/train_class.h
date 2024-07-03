#pragma once

#include "soro/base/soro_types.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/stop_mode.h"

namespace soro::rs {

struct train_class {
  using key = soro::array<char, 3>;
  using nr = uint8_t;

  bool operator==(train_class const&) const = default;

  key key_;

  nr nr_;
  soro::string description_;
  si::accel deacceleration_{si::accel::invalid()};

  si::speed max_speed_{si::speed::invalid()};
  si::speed max_speed_ctc_{si::speed::invalid()};
  rs::stop_mode stop_mode_{stop_mode::passenger};
};

}  // namespace soro::rs