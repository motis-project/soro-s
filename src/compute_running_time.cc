#include "soro/running_time_calculation.h"

#include <complex>
#include <sstream>

#include "utl/verify.h"

#include "soro/parse_train_data.h"

namespace soro {

constexpr auto const DELTA_T = .1F;
constexpr auto const G_M_S2 = 9.81F;
constexpr auto const M_S2_TO_KM_H2 = (1.0F / 1000.0F) * (3600 * 3600);
// constexpr auto const G_KM_H2 = G_M_S2 * M_S2_TO_KM_H2;
constexpr auto const KM_H2_TO_M_S2 = 1.0F / M_S2_TO_KM_H2;
constexpr auto const KILO_NEWTON_TO_NEWTON = 1000.0F;
constexpr auto const M_S_TO_KM_H = 3.6F;
// constexpr auto const H_KM_TO_S_M = 3.6F;
// constexpr auto const H2_KM2_TO_S2_M2 =
//    (3600.0F * 3600.0F) / (1000.0F * 1000.0F);
constexpr auto const BETA = 1.06F;

std::complex<double> analytical(train_physics const& i,
                                std::array<float, 3> const& F,
                                std::complex<double> const base_time,
                                double const base_speed, double const speed) {
  auto C0 = (F[0] * KILO_NEWTON_TO_NEWTON) -
            (i.running_resistance_[0] * i.weight_kg_ * G_M_S2 * KM_H2_TO_M_S2);
  auto C1 = (F[1] * KILO_NEWTON_TO_NEWTON) -
            (i.running_resistance_[1] * i.weight_kg_ * G_M_S2 * KM_H2_TO_M_S2);
  auto C2 = (F[2] * KILO_NEWTON_TO_NEWTON) -
            (i.running_resistance_[2] * G_M_S2 * KM_H2_TO_M_S2);

  C0 /= (i.weight_kg_ * BETA);
  C1 /= (i.weight_kg_ * BETA);
  C2 /= (i.weight_kg_ * BETA);

  auto const o = std::complex<double>(4.0 * C0 * C2 - C1 * C1);
  auto const a = std::atan((C1 + 2.0 * C2 * speed) / std::sqrt(o));
  auto const b = std::atan((C1 + 2.0 * C2 * base_speed) / std::sqrt(o));
  return base_time + (2.0 / std::sqrt(o)) * (a - b);
}

std::string compute_running_time(train_physics const& i) {
  std::stringstream out;
  out << "TIME,DISTANCE,SPEED,ANALYTICAL\n";

  auto coefficients_it = begin(i.tractive_force_);
  auto time_sec = 0.0F;
  auto speed_ms_s = 0.0F;
  auto distance_m = 0.0F;
  auto prev_analytical_time = std::complex<double>{};
  auto prev_analytical_speed = double{0.0};
  while (true) {
    if (speed_ms_s * M_S_TO_KM_H >= i.max_speed_) {
      break;
    }

    if (speed_ms_s * M_S_TO_KM_H >= coefficients_it->to_) {
      prev_analytical_time =
          analytical(i, coefficients_it->tracitive_force_, prev_analytical_time,
                     prev_analytical_speed, coefficients_it->to_ / 3.6);
      prev_analytical_speed = coefficients_it->to_ / 3.6;
      out << "prev analytical time: " << prev_analytical_time.real()
          << ", prev_analytical_speed: " << (prev_analytical_speed * 3.6)
          << "\n";
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

    std::cout << "(((" << coefficients_it->tracitive_force_[0] << " + "
              << coefficients_it->tracitive_force_[1] << " * v + "
              << coefficients_it->tracitive_force_[2] << " * v^2"
              << ") * " << KILO_NEWTON_TO_NEWTON << ") - ";
    std::cout << "((" << i.running_resistance_[0] << " *  " << i.weight_kg_
              << " * " << G_M_S2 << " + "  //
              << i.running_resistance_[1] << " * " << i.weight_kg_ << " * "
              << G_M_S2 << " * v + "  //
              << i.running_resistance_[2] << " * " << G_M_S2 << " * v^2) * "
              << KM_H2_TO_M_S2 << "))";
    std::cout << " / (" << i.weight_kg_ << " * " << BETA << ")\n";

    auto const acceleration_ms_s =
        (tractive_force_newton - resistive_force_newton) /
        (i.weight_kg_ * BETA);

    speed_ms_s += static_cast<float>(acceleration_ms_s * DELTA_T);
    distance_m += speed_ms_s * DELTA_T * 0.5F;
    time_sec += DELTA_T;

    auto const analytical_time =
        analytical(i, coefficients_it->tracitive_force_, prev_analytical_time,
                   prev_analytical_speed, speed_ms_s);

    out << time_sec << "," << distance_m << "," << (speed_ms_s * M_S_TO_KM_H)
        << "," << analytical_time.real() << "," << analytical_time.imag()
        << "\n";
  }

  return out.str();
}

}  // namespace soro