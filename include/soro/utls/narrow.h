#pragma once

#include <concepts>
#include <limits>
#include <type_traits>

#include "soro/utls/sassert.h"

namespace soro::utls {

namespace detail {

// is_narrowing

// signed to unsigned always narrowing
template <std::unsigned_integral Out, std::signed_integral In>
constexpr bool is_narrowing() {
  return true;
}

// unsigned to signed
template <std::signed_integral Out, std::unsigned_integral In>
constexpr bool is_narrowing() {
  return sizeof(In) >= sizeof(Out);
}

// same signedness
template <std::integral Out, std::integral In>
  requires(std::is_signed_v<Out> == std::is_signed_v<In>)
constexpr bool is_narrowing() {
  return sizeof(Out) < sizeof(In);
}

// fits

template <std::integral Out, std::integral In>
  requires(!is_narrowing<Out, In>())
constexpr bool fits(In const) {
  return true;
}

template <std::unsigned_integral Out, std::signed_integral In>
  requires(is_narrowing<Out, In>() && sizeof(Out) < sizeof(In))
constexpr bool fits(In const in) {
  return in >= 0 && in <= std::numeric_limits<Out>::max();
}

template <std::unsigned_integral Out, std::signed_integral In>
  requires(is_narrowing<Out, In>() && sizeof(Out) >= sizeof(In))
constexpr bool fits(In const in) {
  return in >= 0;
}

template <std::signed_integral Out, std::unsigned_integral In>
  requires(is_narrowing<Out, In>())
constexpr bool fits(In const in) {
  return in <= std::numeric_limits<Out>::max();
}

template <std::integral Out, std::integral In>
  requires(is_narrowing<Out, In>() &&
           std::is_signed_v<Out> == std::is_signed_v<In>)
constexpr bool fits(In const in) {
  In const out_max = std::numeric_limits<Out>::max();
  In const out_min = std::numeric_limits<Out>::min();

  return out_min <= in && in <= out_max;
}

}  // namespace detail

template <std::integral Out, std::integral In>
[[nodiscard]] constexpr Out narrow(In const i) {
  utls::sassert(detail::fits<Out>(i), "failed narrowing conversion for {}", i);

  return static_cast<Out>(i);
}

}  // namespace soro::utls