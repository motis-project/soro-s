#pragma once

#include "soro/simulation/dpd.h"
#include "soro/simulation/sim_graph.h"
#include "soro/utls/unixtime.h"

namespace soro::simulation {

inline DurationDPD get_halt_distribution() {
  using namespace soro::utls::literals;

  DurationDPD result;

  constexpr const probability_t p_halt = 0.75;
  constexpr utls::duration const max_trials = 5_m;

  for (auto delay = 0_s; delay < max_trials; delay += 1_s) {
    auto delay_as_prob = static_cast<probability_t>(delay.d_);

    probability_t const prob =
        std::pow(1 - p_halt, delay_as_prob - probability_t(1.0)) * p_halt;

    result.insert(delay, prob);
  }

  return result;
}

inline TimeSpeedDPD get_runtime_distribution(utls::unixtime const& arrival,
                                             kilometer_per_hour const& speed) {
  using namespace soro::utls::literals;

  TimeSpeedDPD result;

  utls::duration const time_variance = 5_m;

  kilometer_per_hour const max_speed_variance = kilometer_per_hour{5};
  kilometer_per_hour const speed_variance = std::min(speed, max_speed_variance);

  const probability_t p_departure = 0.75;

  for (auto new_speed = speed - speed_variance;
       new_speed <= speed + speed_variance; ++new_speed) {
    for (auto delay = 0_s; delay < time_variance; delay += 1_s) {
      auto const delay_as_prob = static_cast<probability_t>(delay.d_);

      probability_t const prob =
          (std::pow(1 - p_departure, delay_as_prob - probability_t(1.0)) *
           p_departure) /
          static_cast<probability_t>(speed_variance.km_h_) * 2;

      result.insert(arrival + delay, new_speed, prob);
    }
  }

  return result;
}

struct disruption {};

}  // namespace soro::simulation