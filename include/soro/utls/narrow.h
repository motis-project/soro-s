#pragma once

#include <type_traits>

#include "soro/utls/concepts/arithmetic.h"
#include "soro/utls/sassert.h"

namespace soro::utls {

namespace detail {

template <utls::arithmetic Out, utls::arithmetic In>
[[nodiscard]] constexpr bool is_narrowing() {
  auto const smaller = sizeof(Out) <= sizeof(In);
  auto const diff_signedness = std::is_signed_v<Out> != std::is_signed_v<In>;

  return smaller || diff_signedness;
}

template <typename Out, typename In>
[[nodiscard]] constexpr bool fits(In&& in) {
  if constexpr (!is_narrowing<Out, In>()) {
    return true;
  }

  if (std::is_signed_v<In> && !std::is_signed_v<Out> && in < 0) {
    return false;
  }

  In const out_max = std::numeric_limits<Out>::max();
  In const out_min = std::numeric_limits<Out>::min();

  return out_min <= in && in <= out_max;
}

}  // namespace detail

template <typename Out, typename In>
[[nodiscard]] constexpr Out narrow(In in) {
  using out_t = std::remove_cvref_t<Out>;
  using in_t = std::remove_cvref_t<In>;

  utls::sassert(detail::fits<out_t, in_t>(in),
                "narrowing conversion from {} to {}", typeid(in_t).name(),
                typeid(out_t).name());

  return static_cast<Out>(in);
}

}  // namespace soro::utls