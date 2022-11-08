#include "soro/utls/coordinates/gps.h"

#include <cmath>

#include "soro/utls/coordinates/angle.h"
#include "soro/utls/coordinates/cartesian.h"

namespace soro::utls {

bool gps::valid() const {
  return std::abs(lat_) <= 360.0 && std::abs(lon_) <= 360.0;
}

// cartesian gps::to_cartesian() const {
//   auto const lon_in_rad = to_rad(lon_);
//   auto const lat_in_rad = to_rad(lat_);
//
//   auto const e2 = 1.0 - std::pow(1.0 - (1.0 / RECIPROCAL_FLATTENING), 2.0);
//   auto const n =
//       EARTH_RADIUS / std::sqrt(1 - e2 * std::pow(std::sin(lat_in_rad), 2));
//   auto const r = n * std::cos(lat_in_rad);
//   auto const x = r * std::cos(lon_in_rad);
//   auto const y = r * std::sin(lon_in_rad);
//   auto const z = (n * (1 - e2)) * std::sin(lat_in_rad);
//
//   return {.x_ = x, .y_ = y, .z_ = z};
// }

// TODO(julian) this is not the correct formula (see above)
// but produces good results for now
cartesian gps::to_cartesian() const {
  return {.x_ = EARTH_RADIUS * std::cos(to_rad(lat_)) * std::cos(to_rad(lon_)),
          .y_ = EARTH_RADIUS * std::cos(to_rad(lat_)) * std::sin(to_rad(lon_)),
          .z_ = EARTH_RADIUS * std::sin(to_rad(lat_))};
}

gps::precision gps::distance(gps const& other) const {
  using std::pow;

  auto const delta_lon = to_rad(other.lon_ - lon_);
  auto const delta_lat = to_rad(other.lat_ - lat_);

  auto const a = pow(sin(delta_lat / 2), 2) + pow(sin(delta_lon / 2), 2) *
                                                  cos(to_rad(lat_)) *
                                                  cos(to_rad(other.lat_));
  auto const c = 2 * atan2(sqrt(a), sqrt(1 - a));

  return EARTH_RADIUS * c;
}

gps::precision gps::bearing(gps const& other) const {
  auto const lon1 = lon_;
  auto const lat1 = to_rad(lat_);
  auto const lon2 = other.lon_;
  auto const lat2 = to_rad(other.lat_);

  auto const delta_lon = to_rad(lon2 - lon1);
  auto const y = sin(delta_lon) * cos(lat2);
  auto const x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(delta_lon);

  return to_deg(atan2(y, x));
}

gps gps::move(gps::precision const distance,
              gps::precision const bearing) const {
  auto const rads_x = to_rad(lon_);
  auto const rads_y = to_rad(lat_);

  auto const bearing_rad = to_rad(bearing);

  auto const cos_r = std::cos(distance / EARTH_RADIUS);
  auto const sin_r = std::sin(distance / EARTH_RADIUS);

  auto moved_lat = std::asin(std::sin(rads_y) * cos_r +
                             std::cos(rads_y) * sin_r * std::cos(bearing_rad));
  auto moved_lon =
      rads_x +
      std::atan2(std::sin(bearing_rad) * std::sin(distance / EARTH_RADIUS) *
                     std::cos(rads_y),
                 cos_r - std::sin(rads_y) * std::sin(moved_lat));

  return {.lon_ = to_deg(moved_lon), .lat_ = to_deg(moved_lat)};
}

}  // namespace soro::utls