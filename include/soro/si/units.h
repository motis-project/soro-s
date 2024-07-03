#pragma once

#include "soro/utls/strong_types/fraction.h"
#include "soro/utls/strong_types/type_list.h"

#include "soro/utls/template/always_false.h"

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
using accel = decltype(std::declval<speed>() / std::declval<time>());
using force = decltype(std::declval<weight>() * std::declval<accel>());
using area = decltype(std::declval<length>() * std::declval<length>());

// -- angle units -- //

using degree = utls::fraction<utls::type_list<utls::degree_tag>,
                              utls::type_list<>, precision>;
using radian = utls::fraction<utls::type_list<utls::radian_tag>,
                              utls::type_list<>, precision>;

// --- various types --- //

using slope = radian;

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

constexpr accel from_m_s2(precision const acc_in_ms2) {
  return accel{acc_in_ms2};
}

constexpr slope from_per_mille_gradient(precision const slope_in_per_mille) {
  return si::slope{std::atan(slope_in_per_mille / 1000.0)};
}

// --- accessor methods --- //

constexpr precision as_s(time const t) { return t.val_; }
constexpr precision as_kg(weight const w) { return w.val_; }
constexpr precision as_ton(weight const w) { return w.val_ / 1000.0; }
constexpr precision as_m_s(speed const l) { return l.val_; }
constexpr precision as_km_h(speed const s) { return s.val_ * 3.6; }
constexpr precision as_m(length const l) { return l.val_; }
constexpr precision as_m_s2(accel const a) { return a.val_; }
constexpr precision as_precision(unitless const u) { return u.val_; }
constexpr precision as_n(force const f) { return f.val_; }
constexpr precision as_kn(force const f) { return f.val_ / 1000.0; }

constexpr precision as_radian(radian const r) { return r.val_; }

template <typename T>
constexpr precision as_si(T const v) {
  if constexpr (std::is_same_v<T, si::length>) {
    return as_m(v);
  } else if constexpr (std::is_same_v<T, si::time>) {
    return as_s(v);
  } else if constexpr (std::is_same_v<T, si::weight>) {
    return as_kg(v);
  } else if constexpr (std::is_same_v<T, si::speed>) {
    return as_m_s(v);
  } else if constexpr (std::is_same_v<T, si::accel>) {
    return as_m_s2(v);
  } else if constexpr (std::is_same_v<T, si::force>) {
    return as_n(v);
  } else if constexpr (std::is_same_v<T, si::unitless>) {
    return as_precision(v);
  } else if constexpr (std::is_same_v<T, si::radian>) {
    return as_radian(v);
  } else {
    static_assert(utls::always_false_v<T>);
  }
}

template <typename To, typename T>
constexpr To from_si(T const v) {
  if constexpr (std::is_same_v<To, si::length>) {
    return from_m(v);
  } else if constexpr (std::is_same_v<To, si::time>) {
    return from_s(v);
  } else if constexpr (std::is_same_v<To, si::weight>) {
    return from_kg(v);
  } else if constexpr (std::is_same_v<To, si::speed>) {
    return from_m_s(v);
  } else if constexpr (std::is_same_v<To, si::accel>) {
    return from_m_s2(v);
  } else if constexpr (std::is_same_v<To, si::force>) {
    return from_n(v);
  } else if constexpr (std::is_same_v<To, si::unitless>) {
    return from_precision(v);
  } else if constexpr (std::is_same_v<To, si::radian>) {
    return from_radian(v);
  } else {
    static_assert(utls::always_false_v<To>);
  }
}

}  // namespace soro::si
