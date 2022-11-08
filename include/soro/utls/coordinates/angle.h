#pragma once

#include <concepts>

#include "soro/si/constants.h"

namespace soro::utls {

constexpr auto to_rad(std::floating_point auto const deg) {
  return deg * PI / decltype(deg){180.0};
}

constexpr auto to_deg(std::floating_point auto const rad) {
  return rad * decltype(rad){180.0} / PI;
}

}  // namespace soro::utls