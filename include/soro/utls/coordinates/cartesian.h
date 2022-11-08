#pragma once

#include "soro/utls/coordinates/coordinates.h"

namespace soro::utls {

struct gps;

struct cartesian {
  using precision = coordinate_precision;

  gps to_gps() const;

  precision x_{0.0};
  precision y_{0.0};
  precision z_{0.0};
};
}  // namespace soro::utls
