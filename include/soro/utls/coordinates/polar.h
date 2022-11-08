#pragma once

#include "soro/utls/coordinates/angle.h"
#include "soro/utls/coordinates/coordinates.h"

namespace soro::utls {

struct polar {
  using precision = coordinate_precision;

  polar(precision const x, precision const y)
      : dist_{std::sqrt(std::pow(x, 2) + std::pow(y, 2))},
        bearing_{to_deg(std::atan2(y, -x))} {
    bearing_ = bearing_ <= 0 ? std::abs(bearing_) : 360.0 - bearing_;
  }

  precision dist_;
  precision bearing_;
};

}  // namespace soro::utls
