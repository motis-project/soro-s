#pragma once

#include <cmath>
#include <limits>

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

struct coordinates {
  using precision = double;

  static constexpr precision invalid() {
    return std::numeric_limits<precision>::quiet_NaN();
  }

  static constexpr coordinates max() {
    return {std::numeric_limits<precision>::max(),
            std::numeric_limits<precision>::max()};
  }

  static constexpr coordinates min() {
    return {std::numeric_limits<precision>::min(),
            std::numeric_limits<precision>::min()};
  }

  coordinates operator+(coordinates const& o) const;
  coordinates operator-(coordinates const& o) const;

  coordinates& operator+=(coordinates const& o);
  coordinates& operator-=(coordinates const& o);

  bool valid() const;

  precision x_{invalid()};
  precision y_{invalid()};
};

struct layout {
  soro::vector_map<station::id, coordinates> station_coordinates_;
  soro::vector_map<element::id, coordinates> element_coordinates_;
  soro::vector_map<node::id, coordinates> node_coordinates_;
};

}  // namespace soro::infra
