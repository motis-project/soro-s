#pragma once

#include "soro/utls/strong_types/fraction.h"
#include "soro/utls/strong_types/type_list.h"

namespace soro::si {

using precision = double;

using unitless =
    utls::fraction<utls::type_list<>, utls::type_list<>, precision>;

// --- base SI units --- //

using length = utls::fraction<utls::type_list<utls::meter_tag>,
                              utls::type_list<>, precision>;
using time = utls::fraction<utls::type_list<utls::second_tag>,
                            utls::type_list<>, precision>;
using weight = utls::fraction<utls::type_list<utls::kilogram_tag>,
                              utls::type_list<>, precision>;
using current = utls::fraction<utls::type_list<utls::ampere_tag>,
                               utls::type_list<>, precision>;

// --- derived SI units --- //

using speed = decltype(std::declval<length>() / std::declval<time>());
using acceleration = decltype(std::declval<speed>() / std::declval<time>());
using force = decltype(std::declval<weight>() * std::declval<acceleration>());
using area = decltype(std::declval<length>() * std::declval<length>());

// --- various types --- //

using per_mille = unitless;

// --- helper constants and methods --- //

template <typename T, std::enable_if_t<utls::is_fraction_v<T>, bool> = true>
T const ZERO = T{precision{0.0}};

template <typename T, std::enable_if_t<utls::is_fraction_v<T>, bool> = true>
T const INVALID = T{std::numeric_limits<precision>::quiet_NaN()};

template <typename T, std::enable_if_t<utls::is_fraction_v<T>, bool> = true>
inline bool valid(T const& t) {
  return !std::isnan(t.val_);
}

// --- factory methods --- //

constexpr length from_m(precision length_in_m) { return length{length_in_m}; };

constexpr length from_km(precision length_in_km) {
  return from_m(length_in_km * 1000.0);
};

constexpr time from_s(precision time_in_s) { return time{time_in_s}; };

constexpr speed from_m_s(precision const speed_in_m_s) {
  return speed{speed_in_m_s};
}

constexpr speed from_km_h(precision const speed_in_km_h) {
  return from_m_s(speed_in_km_h / 3.6);
}

constexpr weight from_kg(precision const weight_in_kg) {
  return weight{weight_in_kg};
}

constexpr weight from_ton(precision const weight_in_ton) {
  return from_kg(weight_in_ton * 1000.0);
}

constexpr acceleration from_m_s2(precision const acc_in_ms2) {
  return acceleration{acc_in_ms2};
}

// --- accessor methods --- //

constexpr precision as_s(time const t) { return t.val_; }
constexpr precision as_ton(weight const w) { return w.val_ / 1000.0; }
constexpr precision as_km_h(speed const s) { return s.val_ * 3.6; }
constexpr precision as_m(length const l) { return l.val_; }
constexpr precision as_precision(unitless const u) { return u.val_; }

}  // namespace soro::si
