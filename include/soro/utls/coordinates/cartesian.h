#pragma once

#include "soro/utls/coordinates/coordinates.h"

namespace soro::utls {

struct gps;

struct cartesian {
  using precision = coordinate_precision;

  gps to_gps() const;

  cartesian& operator+=(cartesian const& other);
  cartesian operator+(cartesian const& other) const;

  cartesian& operator*=(precision const v);
  cartesian operator*(precision const v) const;

  cartesian& operator*=(cartesian const& other);
  cartesian operator*(cartesian const& other) const;

  precision x_{0.0};
  precision y_{0.0};
  precision z_{0.0};
};

cartesian operator*(cartesian::precision const v, cartesian const& c);

}  // namespace soro::utls
