#pragma once

#include <cassert>
#include <cmath>
#include <concepts>

namespace soro {

template <std::floating_point T>
constexpr T const FP_PRECISION = T(0.01);

template <std::floating_point T>
constexpr bool equal(T const a, T const b) noexcept {
  assert(!std::isnan(a));
  assert(!std::isnan(b));

  return std::abs(a - b) < FP_PRECISION<T>;
}

template <std::floating_point T>
constexpr bool zero(T const fp) noexcept {
  return equal(T(0.0), fp);
}

}  // namespace soro
