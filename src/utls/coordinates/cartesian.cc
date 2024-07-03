#include "soro/utls/coordinates/cartesian.h"

#include <cmath>

#include "soro/utls/coordinates/angle.h"
#include "soro/utls/coordinates/coordinates.h"
#include "soro/utls/coordinates/gps.h"

namespace soro::utls {

gps cartesian::to_gps() const {
  gps::precision const lat = std::asin(z_ / EARTH_RADIUS);
  gps::precision const lon = std::atan2(y_, x_);

  return {.lon_ = to_deg(lon), .lat_ = to_deg(lat)};
}

cartesian& cartesian::operator+=(cartesian const& other) {
  x_ += other.x_;
  y_ += other.y_;
  z_ += other.z_;

  return *this;
}

cartesian cartesian::operator+(cartesian const& other) const {
  auto copy = *this;
  copy += other;
  return copy;
}

cartesian& cartesian::operator*=(precision const v) {
  x_ *= v;
  y_ *= v;
  z_ *= v;

  return *this;
}

cartesian cartesian::operator*(precision const v) const {
  auto copy = *this;
  copy *= v;
  return copy;
}

cartesian& cartesian::operator*=(cartesian const& other) {
  x_ *= other.x_;
  y_ *= other.y_;
  z_ *= other.z_;

  return *this;
}

cartesian cartesian::operator*(cartesian const& other) const {
  auto copy = *this;
  copy *= other;
  return copy;
}

cartesian operator*(cartesian::precision const v, cartesian const& c) {
  return c * v;
}

}  // namespace soro::utls
