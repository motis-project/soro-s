#pragma once

#include "fmt/format.h"

#include "soro/si/constants.h"
#include "utl/verify.h"

#include "soro/utls/math/piecewise_function.h"
#include "soro/utls/math/polynomial.h"

#include "soro/base/soro_types.h"
#include "soro/rolling_stock/safety_systems.h"
#include "soro/si/units.h"

namespace soro::rs {

using tractive_force_1_t = si::force;  // [kg * m / s²], newton
using tractive_force_2_t =  // [N * s / m], multiplied with [m / s], newton
    decltype((std::declval<si::force>() * std::declval<si::time>()) /
             std::declval<si::length>());
using tractive_force_3_t =  // [N * s² / m²], multiplied with [m² / s²], newton
    decltype((std::declval<si::force>() * std::declval<si::time>() *
              std::declval<si::time>()) /
             (std::declval<si::length>() * std::declval<si::length>()));

using tractive_polynomial_t =
    soro::utls::polynomial<tractive_force_3_t, tractive_force_2_t,
                           tractive_force_1_t>;

using tractive_piece_t = soro::utls::piece<tractive_polynomial_t, si::speed>;
using tractive_curve_t = soro::utls::piecewise_function<tractive_piece_t>;

using electric_configuration_key = soro::array<char, 2>;
using electric_configuration_t = soro::strong<uint8_t, struct _electric_config>;
using electric_configuration = soro::optional<electric_configuration_t>;

using rolling_resistance_t = si::force;  // [kg * m / s²], newton
using dampening_resistance_t =  // [kg / s], multiplied with [m / s], newton
    decltype(std::declval<si::weight>() / std::declval<si::time>());
using drag_coefficient_t =  // [kg / m], multiplied with [m² / s²] , newton
    decltype(std::declval<si::weight>() / std::declval<si::length>());

using resistance_curve_t =
    soro::utls::polynomial<drag_coefficient_t, dampening_resistance_t,
                           rolling_resistance_t>;

// --- export functions from SI units ---

// --- for tractive forces ---

constexpr si::precision as_kn_h_per_km(tractive_force_2_t const& t2) {
  // does [N * s / m] -> [kN * h / km]
  return t2.val_ / (1000.0 * 3.6);
}

constexpr si::precision as_kn_h2_per_km2(tractive_force_3_t const& t3) {
  // does [N * s / m] -> [kN * h² / km²]
  return t3.val_ / (1000.0 * 3.6 * 3.6);
}

// --- for running resistances --- //

constexpr si::precision as_per_mille(rolling_resistance_t const& r,
                                     si::weight const& train_weight) {
  auto const base = r / (si::GRAVITATIONAL * train_weight);
  return base.val_ * 1000.0;
}

constexpr si::precision as_h_km(dampening_resistance_t const& d,
                                si::weight const train_weight) {
  auto const base = d / (train_weight * si::GRAVITATIONAL);
  return base.val_ * 1000.0 / 3.6;
}

inline si::precision as_kg_h_km(drag_coefficient_t const& d) {
  auto const base = d / si::GRAVITATIONAL;
  return base.val_ / std::pow(3.6, 2);
}

namespace detail {

using train_series_variant =
    soro::strong<uint8_t, struct _train_series_variant>;

using train_series_type = soro::strong<uint8_t, struct _train_series_type>;

struct train_series_key {
  using company = soro::strong<soro::string, struct _train_series_company>;
  using number = soro::strong<soro::string, struct _train_series_number>;

  bool operator==(train_series_key const& o) const = default;
  bool operator<(train_series_key const& o) const;

  number number_;
  company company_;
};

}  // namespace detail

struct traction_unit {
  struct key {
    detail::train_series_key train_series_key_;
    detail::train_series_variant variant_;
  };

  struct equipment {
    using idx = soro::strong<uint8_t, struct _equipment_idx>;

    electric_configuration current_;
    tractive_curve_t curve_;
  };

  bool is_electric() const;
  bool is_diesel() const;

  si::weight translatory_mass() const;

  equipment::idx get_equipment_idx(electric_configuration const c) const;

  key key_;
  detail::train_series_type type_{detail::train_series_type::invalid()};

  soro::string name_;

  si::weight weight_{si::weight::invalid()};
  si::length length_{si::length::invalid()};
  si::unitless mass_factor_{si::unitless::invalid()};
  si::speed max_speed_{si::speed::invalid()};

  PZB pzb_{PZB::NO};
  LZB lzb_{LZB::NO};

  soro::vector_map<equipment::idx, equipment> equipments_;
  resistance_curve_t resistance_curve_;

  soro::map<soro::string, si::weight> brake_weights_;
};

struct train_series {
  using variant = detail::train_series_variant;

  using type_key = soro::array<char, 2>;
  using type = detail::train_series_type;

  using key = detail::train_series_key;

  key key_;
  type type_{type::invalid()};
  soro::map<variant, traction_unit> variants_;
};

}  // namespace soro::rs
