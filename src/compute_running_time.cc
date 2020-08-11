#include "soro/running_time_calculation.h"

#include <complex>
#include <iostream>
#include <sstream>

#include "utl/verify.h"

#include "soro/parse_train_data.h"

namespace soro {

constexpr auto const DELTA_T = .1F;
constexpr auto const G_M_S2 = 9.81F;
constexpr auto const M_S_TO_KM_H = 3.6F;
constexpr auto const BETA = 1.06F;

std::complex<double> analytical(train_physics const& i,
                                std::array<float, 3> const& F,
                                std::complex<double> const base_time,
                                double const base_speed, double const speed) {
  auto const C0 = (F[0] - (i.running_resistance_[0] * i.weight_t_ * G_M_S2)) /
                  (i.weight_t_ * BETA);
  auto const C1 = (F[1] - (i.running_resistance_[1] * i.weight_t_ / 1000)) /
                  (i.weight_t_ * BETA);
  auto const C2 = (F[2] - (i.running_resistance_[2] * G_M_S2 / 1000)) /
                  (i.weight_t_ * BETA);

  auto const o = std::complex<double>(4.0 * C0 * C2 - C1 * C1);
  auto const a =
      std::atan(std::complex<double>{C1 + 2.0 * C2 * speed} / std::sqrt(o));
  auto const b = std::atan(std::complex<double>{C1 + 2.0 * C2 * base_speed} /
                           std::sqrt(o));

  return base_time + (2.0 / (3.6 * std::sqrt(o))) * (a - b);
}

std::string compute_running_time(train_physics const& i) {
  std::stringstream out;
  out << "TIME,DISTANCE,SPEED,ANALYTICAL\n";

  auto coefficients_it = begin(i.tractive_force_);
  auto time_sec = 0.0F;
  auto speed_ms_s = 0.0F;
  auto distance_m = 0.0F;
  auto prev_analytical_time = double{0.0};
  auto prev_analytical_speed = double{0.0};
  while (true) {
    if (speed_ms_s * M_S_TO_KM_H >= i.max_speed_) {
      break;
    }

    if (speed_ms_s * M_S_TO_KM_H >= coefficients_it->to_) {
      prev_analytical_time =
          analytical(i, coefficients_it->tracitive_force_, prev_analytical_time,
                     prev_analytical_speed, coefficients_it->to_)
              .real();
      prev_analytical_speed = coefficients_it->to_;
      ++coefficients_it;
      utl::verify(coefficients_it != end(i.tractive_force_),
                  "no force coefficient for {}", speed_ms_s);
    }

    auto const& F = coefficients_it->tracitive_force_;
    auto const& c = i.running_resistance_;

    auto const speed_km_h = speed_ms_s * 3.6;
    auto const tractive_force_kilo_newton =
        F[0] + F[1] * speed_km_h + F[2] * speed_km_h * speed_km_h;
    auto const resistive_force_kilo_newton =
        c[0] * i.weight_t_ * G_M_S2 +  //
        c[1] * i.weight_t_ * speed_km_h / 1000 +
        c[2] * G_M_S2 * speed_km_h * speed_km_h / 1000.0;
    auto const acceleration_ms_s =
        (tractive_force_kilo_newton - resistive_force_kilo_newton) /
        (i.weight_t_ * BETA);

    auto const C0 =
        (F[0] - (c[0] * i.weight_t_ * G_M_S2)) / (i.weight_t_ * BETA);
    auto const C1 = (F[1] - (c[1] * i.weight_t_ / 1000)) / (i.weight_t_ * BETA);
    auto const C2 = (F[2] - (c[2] * G_M_S2 / 1000)) / (i.weight_t_ * BETA);

    std::cout << "v = " << speed_km_h << " km/h\n";
    std::cout << "w = " << i.weight_t_ << " t\n";
    std::cout << "Z_0 = " << coefficients_it->tracitive_force_[0]
              << " kN (F_0 = " << coefficients_it->tracitive_force_[0] << ")\n";
    std::cout << "Z_1 = " << (coefficients_it->tracitive_force_[1] * speed_km_h)
              << " kN (F_1 = " << coefficients_it->tracitive_force_[1] << ")\n";
    std::cout << "Z_2 = "
              << (coefficients_it->tracitive_force_[2] * speed_km_h *
                  speed_km_h)
              << " kN (F_2 = " << coefficients_it->tracitive_force_[2] << ")\n";
    std::cout << "W_0 = " << (i.running_resistance_[0] * i.weight_t_ * G_M_S2)
              << " kN (c_0 = " << i.running_resistance_[0] << ")\n";
    std::cout << "W_1 = "
              << (i.running_resistance_[1] * i.weight_t_ * speed_km_h)
              << " kN (c_1 = " << i.running_resistance_[1] << ")\n";
    std::cout << "W_2 = "
              << (i.running_resistance_[2] * G_M_S2 * speed_km_h * speed_km_h /
                  1000.0)
              << " kN (c_2 = " << i.running_resistance_[2] << ")\n";
    std::cout << "SUM Z = " << tractive_force_kilo_newton << "\n";
    std::cout << "SUM W = " << resistive_force_kilo_newton << "\n";
    std::cout << "(Z-W)/(m*beta) = " << acceleration_ms_s << "\n";
    std::cout << "C0 + C1*v + C2*v^2 = "
              << (C0 + C1 * speed_km_h + C2 * speed_km_h * speed_km_h) << "\n";
    std::cout << "\n";

    auto const prev_speed = speed_ms_s;
    speed_ms_s += static_cast<float>(acceleration_ms_s * DELTA_T);
    distance_m += (speed_ms_s + prev_speed) * 0.5F * DELTA_T * 0.5F;
    time_sec += DELTA_T;

    auto const analytical_time =
        analytical(i, coefficients_it->tracitive_force_, prev_analytical_time,
                   prev_analytical_speed, speed_km_h);

    out << time_sec << "," << distance_m << "," << (speed_ms_s * M_S_TO_KM_H)
        << "," << analytical_time.real() << "," << analytical_time.imag()
        << "\n";
  }

  return out.str();
}

}  // namespace soro