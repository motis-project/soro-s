#pragma once

#include "soro/utls/std_wrapper/fill.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/physics/rk4/detail/polynom.h"
#include "soro/runtime/physics/rk4/detail/rk4_step.h"

namespace soro::runtime::rk4::detail {

template <std::size_t Degree>
struct sampling_points {
  sampling_points() = delete;
  sampling_points(train_state const init, train_state const current,
                  si::slope const slope, rs::train_physics const& tp) {
    utls::fill(time_, si::as_si(init.time_));
    utls::fill(dist_, si::as_si(init.dist_));
    utls::fill(speed_, si::as_si(init.speed_));

    // initialize time polynom with uniformly
    auto const delta = (si::as_si(current.time_ - init.time_)) / Degree;
    for (auto i = 0U; i < time_.size(); ++i) {
      time_[i] = si::as_si(init.time_) + delta * i;
    }

    // initialize distance and speed polys with the last two steps of rk4
    dist_[Degree] = si::as_si(current.dist_);
    speed_[Degree] = si::as_si(current.speed_);

    for (auto i = 1U; i < Degree; ++i) {
      speed_[i] = speed_[i - 1];
      dist_[i] = dist_[i - 1];

      // delta_t as fixed stepsize h
      auto const step_size = si::from_si<si::time>(time_[i] - time_[i - 1]);
      auto const rk_result =
          rk4_step(si::from_si<si::speed>(speed_[i]), step_size, slope, tp);

      speed_[i] += si::as_si(rk_result.speed_);
      dist_[i] += si::as_si(rk_result.dist_);
    }
  }

  template <typename T>
  auto get() const {
    if constexpr (std::is_same_v<T, si::time>) {
      return time_;
    } else if constexpr (std::is_same_v<T, si::length>) {
      return dist_;
    } else if constexpr (std::is_same_v<T, si::speed>) {
      return speed_;
    }
  }

  polynom<si::time, Degree> time_;
  polynom<si::length, Degree> dist_;
  polynom<si::speed, Degree> speed_;
};

}  // namespace soro::runtime::rk4::detail