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
constexpr bool fits(In&&) {
  return true;
}

template <std::unsigned_integral Out, std::signed_integral In>
  requires(is_narrowing<Out, In>() && sizeof(Out) < sizeof(In))
constexpr bool fits(In&& in) {
  return in >= 0 && in <= std::numeric_limits<Out>::max();
}

template <std::unsigned_integral Out, std::signed_integral In>
  requires(is_narrowing<Out, In>() && sizeof(Out) >= sizeof(In))
constexpr bool fits(In&& in) {
  return in >= 0;
}

template <std::signed_integral Out, std::unsigned_integral In>
  requires(is_narrowing<Out, In>())
constexpr bool fits(In&& in) {
  return in <= std::numeric_limits<Out>::max();
}

template <std::integral Out, std::integral In>
  requires(is_narrowing<Out, In>() &&
           std::is_signed_v<Out> == std::is_signed_v<In>)
constexpr bool fits(In&& in) {
  In const out_max = std::numeric_limits<Out>::max();
  In const out_min = std::numeric_limits<Out>::min();

  return out_min <= in && in <= out_max;
}

}  // namespace detail

template <std::integral Out, std::integral In>
[[nodiscard]] constexpr Out narrow(In&& in) {
  utls::sassert(detail::fits<Out>(std::forward<In>(in)),
                "failed narrowing conversion for {}", in);

  return static_cast<Out>(std::forward<In>(in));
}

}  // namespace soro::utls