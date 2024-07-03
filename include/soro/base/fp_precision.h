#pragma once

#include <cassert>
#include <cmath>
#include <concepts>

#include "soro/utls/sassert.h"

namespace soro {

template <std::floating_point T>
constexpr T const FP_PRECISION = T(0.0001);

template <std::floating_point T>
constexpr bool equal(T const a, T const b) noexcept {
  utls::sassert(!std::isnan(a) && !std::isnan(b), "never compare with nan");
  return std::abs(a - b) < FP_PRECISION<T>;
}

template <std::floating_point T>
constexpr bool zero(T const fp) noexcept {
  return equal(T(0.0), fp);
}

template <std::floating_point T>
constexpr T smooth(T const f) noexcept {
  constexpr auto factor = 1 / FP_PRECISION<T>;
  return std::round(f * factor) / factor;
}

}  // namespace soro
