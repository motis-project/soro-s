#include "soro/running_time_calculation.h"

#include <complex>
#include <sstream>

#include "utl/verify.h"

#include "soro/parse_train_data.h"

namespace soro {

constexpr auto const DELTA_T = 1.5F;
constexpr auto const M_S_TO_KM_H = 3.6F;

float analytical(std::array<float, 3> coefficients, float const t0,
                 float const v0_kmh, float const v1_kmh) {
  auto const& [c0, c1, c2] = coefficients;
  auto const o = std::sqrt(std::complex<float>(4.0F * c0 * c2 - c1 * c1));
  auto const a = std::atan(std::complex<float>{c1 + 2.0F * c2 * v1_kmh} / o);
  auto const b = std::atan(std::complex<float>{c1 + 2.0F * c2 * v0_kmh} / o);
  return t0 + ((2.0F / (3.6F * o)) * (a - b)).real();
}

float time_until_analytical(train_physics const& i,
                            float const target_speed_kmh) {
  auto prev_speed_kmh = 0.0F;
  auto prev_time_s = 0.0F;
  for (auto const& f : i.tractive_force_) {
    if (f.to_ < target_speed_kmh) {
      prev_time_s =
          analytical(f.coefficients_, prev_time_s, prev_speed_kmh, f.to_);
      prev_speed_kmh = f.to_;
    } else {
      return analytical(f.coefficients_, prev_time_s, prev_speed_kmh,
                        target_speed_kmh);
    }
  }
  assert(false);
  throw std::runtime_error{"error in running time calculation"};
}

float time_until_numerical(train_physics const& i,
                           float const target_speed_kmh) {
  auto coefficients_it = begin(i.tractive_force_);
  auto time_sec = 0.0F;
  auto speed_ms_s = 0.0F;
  auto distance_m = 0.0F;
  while (true) {
    if (speed_ms_s * M_S_TO_KM_H >= target_speed_kmh) {
      return time_sec;
    }

    if (speed_ms_s * M_S_TO_KM_H >= coefficients_it->to_) {
      ++coefficients_it;
      if (coefficients_it == end(i.tractive_force_)) {
        assert(false);
        throw std::runtime_error{"error in running time calculation"};
      }
    }

    auto const speed_km_h = speed_ms_s * 3.6;
    auto const acceleration_ms_s =
        coefficients_it->coefficients_[0] +
        coefficients_it->coefficients_[1] * speed_km_h +
        coefficients_it->coefficients_[2] * speed_km_h * speed_km_h;

    speed_ms_s += static_cast<float>(acceleration_ms_s * DELTA_T);
    distance_m += speed_ms_s * DELTA_T * 0.5F;
    time_sec += DELTA_T;
  }
}

std::string compute_running_time(train_physics const& i) {
  std::stringstream out;
  out << "TIME,DISTANCE,SPEED,ANALYTICAL\n";

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

    auto const speed_km_h = speed_ms_s * 3.6F;
    auto const acceleration_ms_s =
        coefficients_it->coefficients_[0] +
        coefficients_it->coefficients_[1] * speed_km_h +
        coefficients_it->coefficients_[2] * speed_km_h * speed_km_h;

    auto const prev_speed = speed_ms_s;
    speed_ms_s += static_cast<float>(acceleration_ms_s * DELTA_T);
    distance_m += (speed_ms_s + prev_speed) * 0.5F * DELTA_T * 0.5F;
    time_sec += DELTA_T;

    out << time_sec << "," << distance_m << "," << (speed_ms_s * M_S_TO_KM_H)
        << "," << time_until_analytical(i, speed_km_h) << "\n";
  }

  return out.str();
}

}  // namespace soro