#pragma once

#include "soro/utls/coordinates/coordinates.h"

namespace soro::utls {

struct cartesian;

struct gps {
  using precision = coordinate_precision;

  static constexpr precision INVALID = -361.0;

  cartesian to_cartesian() const;
  precision distance(gps const& other) const;  // in meters
  precision bearing(gps const& other) const;  // in degrees
  gps move(precision const distance, precision const bearing) const;
  bool valid() const;

  precision lon_{INVALID};
  precision lat_{INVALID};
};

}  // namespace soro::utls
