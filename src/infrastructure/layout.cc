#include "soro/infrastructure/layout.h"

#include <cmath>

namespace soro::infra {

coordinates& coordinates::operator+=(coordinates const& o) {
  x_ += o.x_;
  y_ += o.y_;
  return *this;
}

coordinates coordinates::operator+(coordinates const& o) const {
  auto copy = *this;
  copy += o;
  return copy;
}

coordinates& coordinates::operator-=(coordinates const& o) {
  x_ -= o.x_;
  y_ -= o.y_;
  return *this;
}

coordinates coordinates::operator-(coordinates const& o) const {
  auto copy = *this;
  copy -= o;
  return copy;
}

bool coordinates::valid() const { return !std::isnan(x_) && !std::isnan(y_); }

}  // namespace soro::infra