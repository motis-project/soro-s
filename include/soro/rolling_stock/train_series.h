#pragma once

#include "fmt/format.h"
#include "utl/verify.h"

#include "soro/base/soro_types.h"
#include "soro/si/units.h"
#include "soro/utls/math/piecewise_function.h"
#include "soro/utls/math/polynomial.h"

namespace soro::rs {

using variant_id = uint32_t;

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

using rolling_resistance_t = si::force;  // [kg * m / s²], newton
using dampening_resistance_t =  // [kg / s], multiplied with [m / s], newton
    decltype(std::declval<si::weight>() / std::declval<si::time>());
using drag_coefficient_t =  // [kg / m], multiplied with [m² / s²] , newton
    decltype(std::declval<si::weight>() / std::declval<si::length>());

using resistance_curve_t =
    soro::utls::polynomial<drag_coefficient_t, dampening_resistance_t,
                           rolling_resistance_t>;

struct traction_vehicle {
  soro::string name_;

  si::weight weight_{si::INVALID<si::weight>};
  si::speed max_speed_{si::INVALID<si::speed>};

  si::acceleration deacceleration_ = si::from_m_s2(-0.7);

  tractive_curve_t tractive_curve_;
  resistance_curve_t resistance_curve_;
};

struct train_series {
  soro::string get_key() const {
    return fmt::format("{}-{}", nr_, company_nr_);
  }

  soro::string nr_;
  soro::string company_nr_;

  soro::map<variant_id, traction_vehicle> variants_;
};

}  // namespace soro::rs
