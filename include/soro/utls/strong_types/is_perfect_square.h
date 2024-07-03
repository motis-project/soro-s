#pragma once

#include <cmath>
#include <concepts>

namespace soro::utls {

template <std::integral T>
constexpr bool is_perfect_square(T const i) {
  double const sqrt = std::sqrt(i);
  auto const rounded = std::round(sqrt);
  T const cast = static_cast<T>(rounded);
  return cast * cast == i;
}

}  // namespace soro::utls