#pragma once

#include <algorithm>
#include <limits>

#include "soro/utls/coordinates/coordinates.h"
#include "soro/utls/sassert.h"

#include "soro/utls/concepts/yields.h"

namespace soro::utls {

struct cartesian;

struct gps {
  using value_t = coordinate_precision;
  using precision = value_t;

  static constexpr gps invalid() {
    return {std::numeric_limits<precision>::quiet_NaN(),
            std::numeric_limits<precision>::quiet_NaN()};
  }

  cartesian to_cartesian() const;
  precision distance(gps const& other) const;  // in meters
  precision bearing(gps const& other) const;  // in degrees
  gps move(precision const distance, precision const bearing) const;
  bool valid() const;

  precision lon_{std::numeric_limits<precision>::quiet_NaN()};
  precision lat_{std::numeric_limits<precision>::quiet_NaN()};
};

struct bounding_box {

  bool valid() { return south_west_.valid() && north_east_.valid(); }

  gps south_west_{gps::invalid()};
  gps north_east_{gps::invalid()};
};

template <typename Range>
  requires utls::yields<gps, Range>
bounding_box get_bounding_box(Range&& r) {
  bounding_box bb;

  bb.south_west_.lon_ = std::numeric_limits<gps::value_t>::max();
  bb.south_west_.lat_ = std::numeric_limits<gps::value_t>::max();
  bb.north_east_.lon_ = std::numeric_limits<gps::value_t>::min();
  bb.north_east_.lat_ = std::numeric_limits<gps::value_t>::min();

  for (auto const& p : r) {
    bb.south_west_.lon_ = std::min(bb.south_west_.lon_, p.lon_);
    bb.south_west_.lat_ = std::min(bb.south_west_.lat_, p.lat_);
    bb.north_east_.lon_ = std::max(bb.north_east_.lon_, p.lon_);
    bb.north_east_.lat_ = std::max(bb.north_east_.lat_, p.lat_);
  }

  utls::ensure(bb.valid());

  return bb;
}

}  // namespace soro::utls
