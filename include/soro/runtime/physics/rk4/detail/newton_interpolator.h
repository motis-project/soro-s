#pragma once

#include "soro/utls/narrow.h"

#include "soro/runtime/physics/rk4/detail/polynom.h"
#include "soro/runtime/physics/rk4/detail/sampling_points.h"

namespace soro::runtime::rk4::detail {

template <std::size_t Degree, typename To, typename ToPrime>
struct newton_interpolator {
  newton_interpolator(sampling_points<Degree> const& sps) {
    coefficients_ = sps.template get<To>();
    sampling_points_ = sps.template get<si::length>();

    for (auto i = 0U; i < Degree + 1; ++i) {
      for (auto k = Degree; k >= i + 1; --k) {
        coefficients_[k] = (coefficients_[k] - coefficients_[k - 1]) /
                           (sampling_points_[k] - sampling_points_[k - i - 1]);
      }
    }
  }

  To interpolate(si::length const dist) const {
    utls::expect(dist >= si::length::zero());

    auto result = coefficients_[Degree];
    for (auto idx = utls::narrow<int8_t>(Degree) - 1; idx >= 0; --idx) {
      auto const i = utls::narrow<soro::size_t>(idx);
      result =
          result * (si::as_si(dist) - sampling_points_[i]) + coefficients_[i];
    }

    return si::from_si<To>(result);
  }

  std::pair<To, ToPrime> operator()(si::length const dist) const {
    utls::expect(dist >= si::length::zero());

    auto result = coefficients_[Degree];
    auto derivative = coefficients_[Degree];

    for (auto idx = utls::narrow<int8_t>(Degree) - 1; idx > 0; --idx) {
      auto const i = utls::narrow<soro::size_t>(idx);
      result =
          result * (si::as_si(dist) - sampling_points_[i]) + coefficients_[i];
      derivative =
          derivative * (si::as_si(dist) - sampling_points_[i - 1]) + result;
    }
    result =
        result * (si::as_si(dist) - sampling_points_[0]) + coefficients_[0];

    return {si::from_si<To>(result), ToPrime{derivative}};
  }

  polynom<To, Degree> coefficients_;
  polynom<si::length, Degree> sampling_points_;
};

}  // namespace soro::runtime::rk4::detail