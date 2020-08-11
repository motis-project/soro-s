#include "soro/running_time_calculation.h"

#include <complex>
#include <sstream>

#include "utl/verify.h"

#include "soro/parse_train_data.h"

namespace soro {

constexpr auto const DELTA_T = 1.5F;
constexpr auto const G_M_S2 = 9.81F;
constexpr auto const M_S_TO_KM_H = 3.6F;
constexpr auto const BETA = 1.06F;
constexpr auto const NEWTON_TO_KILO_NEWTON = 1 / 1000.0;

double analytical(train_physics const& i, std::array<float, 3> const& F,
                  double const t0, double const v0_kmh, double const v1_kmh) {
  auto const C0 = (F[0] - (i.running_resistance_[0] * i.weight_t_ * G_M_S2)) /
                  (i.weight_t_ * BETA);
  auto const C1 = (F[1] - (i.running_resistance_[1] * i.weight_t_ * G_M_S2 *
                           NEWTON_TO_KILO_NEWTON)) /
                  (i.weight_t_ * BETA);
  auto const C2 =
      (F[2] - (i.running_resistance_[2] * G_M_S2 * NEWTON_TO_KILO_NEWTON)) /
      (i.weight_t_ * BETA);
  auto const o = std::sqrt(std::complex<double>(4.0 * C0 * C2 - C1 * C1));
  auto const a = std::atan(std::complex<double>{C1 + 2.0 * C2 * v1_kmh} / o);
  auto const b = std::atan(std::complex<double>{C1 + 2.0 * C2 * v0_kmh} / o);
  return t0 + ((2.0 / (3.6 * o)) * (a - b)).real();
}

double time_until(train_physics const& i, double const target_speed_kmh) {
  auto prev_speed_kmh = 0.0;
  auto prev_time_s = 0.0;
  for (auto const& f : i.tractive_force_) {
    if (f.to_ < target_speed_kmh) {
      prev_time_s =
          analytical(i, f.tracitive_force_, prev_time_s, prev_speed_kmh, f.to_);
      prev_speed_kmh = f.to_;
    } else {
      return analytical(i, f.tracitive_force_, prev_time_s, prev_speed_kmh,
                        target_speed_kmh);
    }
  }
  assert(false);
  throw std::runtime_error{"error in running time calculation"};
}

std::string compute_running_time(train_physics const& i) {
  std::stringstream out;
  out << "TIME,DISTANCE,SPEED,ANALYTICAL,ANALYTICAL_1\n";

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

    auto const& z = coefficients_it->tracitive_force_;
    auto const& c = i.running_resistance_;

    auto const speed_km_h = speed_ms_s * 3.6;
    auto const tractive_force_kilo_newton =
        z[0] + z[1] * speed_km_h + z[2] * speed_km_h * speed_km_h;
    auto const resistive_force_kilo_newton =
        c[0] * i.weight_t_ * G_M_S2 +  //
        c[1] * i.weight_t_ * G_M_S2 * speed_km_h * NEWTON_TO_KILO_NEWTON +
        c[2] * G_M_S2 * speed_km_h * speed_km_h * NEWTON_TO_KILO_NEWTON;
    auto const acceleration_ms_s =
        (tractive_force_kilo_newton - resistive_force_kilo_newton) /
        (i.weight_t_ * BETA);

    auto const prev_speed = speed_ms_s;
    speed_ms_s += static_cast<float>(acceleration_ms_s * DELTA_T);
    distance_m += (speed_ms_s + prev_speed) * 0.5F * DELTA_T * 0.5F;
    time_sec += DELTA_T;

    out << time_sec << "," << distance_m << "," << (speed_ms_s * M_S_TO_KM_H)
        << "," << time_until(i, speed_km_h) << "\n";
  }

  return out.str();
}

}  // namespace soro