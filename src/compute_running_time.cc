#include "soro/running_time_calculation.h"

#include <sstream>

#include "utl/verify.h"

#include "soro/parse_train_data.h"

namespace soro {

constexpr auto const DELTA_T = 0.01F;
constexpr auto const G_M_S2 = 9.81F;
constexpr auto const M_S2_TO_KM_H2 = (1.0F / 1000.0F) * (3600 * 3600);
constexpr auto const KM_H2_TO_M_S2 = 1.0F / M_S2_TO_KM_H2;
constexpr auto const KILO_NEWTON_TO_NEWTON = 1000.0F;
constexpr auto const M_S_TO_KM_H = 3.6F;
constexpr auto const BETA = 1.06F;

std::string compute_running_time(train_physics const& i) {
  std::stringstream out;
  out << "TIME,DISTANCE,SPEED\n";

  auto coefficients_it = begin(i.tractive_force_);
  auto time_sec = 0.0F;
  auto speed_ms_s = 0.0F;
  auto distance_m = 0.0F;
  while (true) {
    if (speed_ms_s * M_S_TO_KM_H >= i.max_speed_) {
      break;
    }

    if (speed_ms_s * M_S_TO_KM_H >= coefficients_it->to_) {
      ++coefficients_it;
      utl::verify(coefficients_it != end(i.tractive_force_),
                  "no force coefficient for {}", speed_ms_s);
    }

    auto const speed_km_h = speed_ms_s * 3.6;
    auto const tractive_force_newton =
        (coefficients_it->tracitive_force_[0] +
         coefficients_it->tracitive_force_[1] * speed_km_h +
         coefficients_it->tracitive_force_[2] * speed_km_h * speed_km_h) *
        KILO_NEWTON_TO_NEWTON;
    auto const resistive_force_newton =
        (i.running_resistance_[0] * i.weight_kg_ * G_M_S2 +
         i.running_resistance_[1] * i.weight_kg_ * G_M_S2 * speed_km_h +
         i.running_resistance_[2] * G_M_S2 * speed_km_h * speed_km_h) *
        KM_H2_TO_M_S2;
    auto const acceleration_ms_s =
        (tractive_force_newton - resistive_force_newton) /
        (i.weight_kg_ * BETA);

    speed_ms_s += static_cast<float>(acceleration_ms_s * DELTA_T);
    distance_m += speed_ms_s * DELTA_T * 0.5F;
    time_sec += DELTA_T;

    out << time_sec << "," << distance_m << "," << (speed_ms_s * M_S_TO_KM_H)
        << "\n";
  }

  return out.str();
}

}  // namespace soro