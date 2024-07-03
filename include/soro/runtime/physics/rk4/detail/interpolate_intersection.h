#pragma once

#include "soro/si/units.h"

namespace soro::runtime::rk4::detail {

// Uses Newton's method to interpolate the intersection given by
// f1(x) - f2(x) = 0. To achieve this we need f1(x), f2(x) and f1'(x), f2'(x).
// F1 must supply f1(x) AND f1'(x) and F2 must supply f2(x) AND f2'(x).
// https://en.wikipedia.org/wiki/Newton%27s_method
template <typename F1, typename F2>
si::length interpolate_intersection(si::length const start, F1&& f1, F2&& f2,
                                    si::speed const min_error) {
  utls::expect(start > si::length::zero(), "start must be positive");

  constexpr auto max_iterations = 5U;

  auto x = start;

  for (auto iter = 0U; iter < max_iterations; ++iter) {
    if (x < si::length::zero()) break;

    auto const [f1_x, f1_prime_x] = f1(x);
    auto const [f2_x, f2_prime_x] = f2(x);

    auto const f_x = f1_x - f2_x;

    if (f_x.abs() < min_error) return x;

    auto const f_prime_x = f1_prime_x - f2_prime_x;

    auto const delta_x = f_x / f_prime_x;
    x = x - delta_x;
  }

  return start;
}

}  // namespace soro::runtime::rk4::detail