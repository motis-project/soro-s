#pragma once

#include <algorithm>

#include "soro/utls/coordinates/coordinates.h"

#include "soro/utls/concepts/yields.h"

namespace soro::utls {

struct cartesian;

struct gps {
  using precision = coordinate_precision;

  static constexpr precision INVALID = -361.0;
  static constexpr precision MAX_INVALID = 361.0;

  cartesian to_cartesian() const;
  precision distance(gps const& other) const;  // in meters
  precision bearing(gps const& other) const;  // in degrees
  gps move(precision const distance, precision const bearing) const;
  bool valid() const;

  precision lon_{INVALID};
  precision lat_{INVALID};
};

struct bounding_box {
  gps south_west_{.lon_ = gps::MAX_INVALID, .lat_ = gps::MAX_INVALID};
  gps north_east_{.lon_ = gps::INVALID, .lat_ = gps::INVALID};
};

template <typename Range>
  requires utls::yields<gps, Range>
bounding_box get_bounding_box(Range&& r) {
  bounding_box bb;

  for (auto const& p : r) {
    bb.south_west_.lon_ = std::min(bb.south_west_.lon_, p.lon_);
    bb.south_west_.lat_ = std::min(bb.south_west_.lat_, p.lat_);
    bb.north_east_.lon_ = std::max(bb.north_east_.lon_, p.lon_);
    bb.north_east_.lat_ = std::max(bb.north_east_.lat_, p.lat_);
  }

  return bb;
}

}  // namespace soro::utls
