#include "soro/utls/coordinates/cartesian.h"

#include "soro/utls/coordinates/angle.h"
#include "soro/utls/coordinates/gps.h"

namespace soro::utls {

gps cartesian::to_gps() const {
  gps::precision const lat = std::asin(z_ / EARTH_RADIUS);
  gps::precision const lon = std::atan2(y_, x_);

  return {.lon_ = to_deg(lon), .lat_ = to_deg(lat)};
}

}  // namespace soro::utls
