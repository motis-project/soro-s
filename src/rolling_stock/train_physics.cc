#include "soro/rolling_stock/train_physics.h"

#include <cmath>
#include <algorithm>
#include <utility>

#include "soro/base/soro_types.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/any_of.h"
#include "soro/utls/std_wrapper/count_if.h"
#include "soro/utls/std_wrapper/min_element.h"

#include "soro/si/constants.h"
#include "soro/si/units.h"

#include "soro/rolling_stock/safety_systems.h"
#include "soro/rolling_stock/stop_mode.h"
#include "soro/rolling_stock/train_category.h"
#include "soro/rolling_stock/train_class.h"
#include "soro/rolling_stock/train_series.h"
#include "soro/rolling_stock/train_type.h"

namespace soro::rs {

using namespace soro::utls;

tractive_curve_t configured_traction_unit::traction_curve() const {
  return traction_unit_.equipments_[selected_].curve_;
}

si::force configured_traction_unit::tractive_force(si::speed const v) const {
  return traction_unit_.equipments_[selected_].curve_(v);
}

#if !defined(SERIALIZE)
train_physics::train_physics(
    soro::vector<configured_traction_unit> ctus,
    si::weight const carriage_weight, si::length const carriage_length,
    si::speed const max_speed, soro::size_t const wagons, rs::LZB const lzb,
    rs::stop_mode const stop_mode, rs::brake_position const brake_position,
    rs::brake_type const brake_type,
    brake_weight_percentage const brake_weight_percentage,
    soro::optional<rs::tilt_technology> const tilt_technology,
    air_resistance_coefficient const air_coefficient,
    bearing_friction_coefficient const bearing_coefficient,
    rs::train_class train_class, rs::train_category train_category,
    soro::vector<transportation_specialty> specialties)
    : traction_units_{std::move(ctus)},
      carriage_weight_{carriage_weight},
      carriage_length_{carriage_length},
      max_speed_{max_speed},
      cars_{wagons},
      lzb_{lzb},
      stop_mode_{stop_mode},
      brake_position_{brake_position},
      brake_type_{brake_type},
      percentage_{brake_weight_percentage},
      tilt_technology_{tilt_technology},
      air_resistance_{air_coefficient},
      bearing_friction_{bearing_coefficient},
      train_class_{std::move(train_class)},
      train_category_{std::move(train_category)},
      specialties_{std::move(specialties)} {}
#endif

si::length train_physics::length() const {
  return traction_vehicle_length() + carriage_length();
}

si::length train_physics::traction_vehicle_length() const {
  return accumulate(
      traction_units_, si::length::zero(),
      [&](auto&& acc, auto&& ctu) { return acc + ctu.traction_unit_.length_; });
}

si::length train_physics::carriage_length() const { return carriage_length_; }

si::weight train_physics::weight() const {
  return traction_vehicle_weight() + carriage_weight();
}

si::weight train_physics::traction_vehicle_weight() const {
  return accumulate(
      traction_units_, si::weight::zero(),
      [](auto&& acc, auto&& ctu) { return acc + ctu.traction_unit_.weight_; });
}

si::weight train_physics::carriage_weight() const { return carriage_weight_; }

si::weight train_physics::translatory_weight() const {
  return translatory_vehicle_weight() + translatory_carriage_weight();
}

si::weight train_physics::translatory_vehicle_weight() const {
  return accumulate(traction_units_, si::weight::zero(),
                    [](auto&& acc, auto&& ctu) {
                      return acc + ctu.traction_unit_.translatory_mass();
                    });
}

si::weight train_physics::translatory_carriage_weight() const {
  return carriage_weight_ * carriage_mass_factor;
}

si::speed train_physics::max_speed() const {
  auto const vehicle_max_speed =
      min_element(traction_units_, [](auto&& v1, auto&& v2) {
        return v1.traction_unit_.max_speed_ < v2.traction_unit_.max_speed_;
      })->traction_unit_.max_speed_;
  return std::min(max_speed_, vehicle_max_speed);
}

si::speed train_physics::max_speed(si::speed const speed_limit) const {
  return std::min(max_speed(), speed_limit);
}

si::speed train_physics::approach_speed() const {
  if (percentage_ <= brake_weight_percentage{65}) {
    return si::from_km_h(55);
  } else if (percentage_ <= brake_weight_percentage{110}) {
    return si::from_km_h(70);
  } else {  // > 110
    return si::from_km_h(85);
  }
}

si::force train_physics::tractive_force(si::speed const v) const {
  return accumulate(
      traction_units_, si::force::zero(),
      [&](auto&& acc, auto&& tu) { return acc + tu.tractive_force(v); });
}

si::force train_physics::resistive_force(si::speed const v,
                                         si::slope const slope) const {
  // resistive force given by the vehicles
  auto result = accumulate(
      traction_units_, si::force::zero(), [&](auto&& acc, auto&& ctu) {
        return acc + ctu.traction_unit_.resistance_curve_(v);
      });

  // resistive force given by the cars
  if (train_type() == train_type::freight && !carriage_weight().is_zero()) {
    auto const weight_force = carriage_weight() * si::GRAVITATIONAL;

    si::force const t1 = bearing_friction_ * weight_force;
    si::force const t2 = air_resistance_ * weight_force * v.pow<2>();

    result += t1 + t2;
  } else if (carriage_weight() > si::weight::zero()) {
    auto const weight_force = carriage_weight() * si::GRAVITATIONAL;

    si::force const t1 = (1.9 / 1000.0) * weight_force;

    using t2_coefficient =
        decltype(std::declval<si::time>() / std::declval<si::length>());
    t2_coefficient const t2_coeff{0.0025 * 3.6 / 1000.0};
    si::force const t2 = t2_coeff * weight_force * v;

    using t3_coefficient = decltype(std::declval<si::weight>() *
                                    std::declval<si::time>().pow<2>() /
                                    std::declval<si::length>().pow<2>());
    t3_coefficient const t3_coeff{0.0048 * 1.45 * std::pow(3.6, 2) *
                                  (cars_ + 2.7)};
    si::speed const relative_speed = v + default_wind;
    si::force const t3 = t3_coeff * si::GRAVITATIONAL * relative_speed.pow<2>();

    result += t1 + t2 + t3;
  }

  // force given by the slope of the track
  // might be positive or negative!
  result += std::sin(slope.val_) * weight() * si::GRAVITATIONAL;

  return result;
}

si::accel train_physics::acceleration(si::speed const current_speed,
                                      si::slope const slope) const {
  utls::expect(current_speed >= si::speed::zero(), "negative speed");
  utls::expect(current_speed <= max_speed(), "speed exceeds max speed");

  auto const tractive = tractive_force(current_speed);
  auto const resistive = resistive_force(current_speed, slope);
  auto const result = (tractive - resistive) / translatory_weight();
  return result;
}

si::accel train_physics::natural_accel(si::speed const current_speed,
                                       si::slope const slope) const {
  return -resistive_force(current_speed, slope) / translatory_weight();
}

si::accel default_braking_acceleration(train_physics const& tp) {
  return tp.train_class().deacceleration_;
}

bool requires_secure_deacceleration(train_physics const& tp,
                                    si::speed const infra_limit,
                                    si::speed const bwp_limit) {
  return bwp_limit < tp.max_speed(infra_limit);
}

si::accel train_physics::braking_deaccel(
    si::speed const infra_limit, si::speed const bwp_limit,
    si::length const brake_path_length) const {
  if (requires_secure_deacceleration(*this, infra_limit, bwp_limit)) {
    auto const secure = -(bwp_limit.pow<2>() / (2 * brake_path_length));
    return std::max(secure, default_braking_acceleration(*this));
  } else {
    return default_braking_acceleration(*this);
  }
}

rs::LZB train_physics::lzb() const { return lzb_; }

bool train_physics::has_lzb() const { return lzb_ == LZB::YES; }

stop_mode train_physics::stop_mode() const {
  // explicit stop mode from the timetable overwrites train class stop mode
  //  return train_class_.stop_mode_;
  return stop_mode_;
}

bool train_physics::is_freight() const { return rs::is_freight(train_type()); }

bool train_physics::is_passenger() const {
  return rs::is_passenger(train_type());
}

rs::train_type train_physics::train_type() const {
  return is_passenger_stop_mode(train_class_.stop_mode_)
             ? rs::train_type::passenger
             : rs::train_type::freight;
}

bool train_physics::has_diesel() const {
  return utls::any_of(traction_units_, [](auto&& ctu) {
    return ctu.traction_unit_.is_diesel();
  });
}

bool train_physics::has_electric() const {
  return utls::any_of(traction_units_, [](auto&& ctu) {
    return ctu.traction_unit_.is_electric();
  });
}

rs::brake_type train_physics::brake_type() const { return brake_type_; }

rs::brake_position train_physics::brake_position() const {
  return brake_position_;
}

rs::brake_weight_percentage train_physics::percentage() const {
  return percentage_;
}

soro::size_t train_physics::vehicle_count() const {
  return narrow<soro::size_t>(traction_units_.size());
}

soro::vector<configured_traction_unit> const& train_physics::units() const {
  return traction_units_;
}

soro::size_t train_physics::wagons() const { return cars_; }

rs::bearing_friction_coefficient train_physics::bearing_friction() const {
  return bearing_friction_;
}

rs::air_resistance_coefficient train_physics::air_resistance() const {
  return air_resistance_;
}

soro::optional<tilt_technology> const& train_physics::tilt_technology() const {
  return tilt_technology_;
}

soro::vector<rs::transportation_specialty> const& train_physics::specialties()
    const {
  return specialties_;
}

rs::train_class const& train_physics::train_class() const {
  return train_class_;
}

rs::train_category const& train_physics::train_category() const {
  return train_category_;
}

surcharge_factor::value_type surcharge_percentage_electric(
    train_physics const& tp, si::speed const current_max_velocity) {
  auto const electric_count = utls::count_if(
      tp.units(), [](auto&& ctu) { return ctu.traction_unit_.is_electric(); });

  auto const weight_per_vehicle = tp.weight() / electric_count;

  if (current_max_velocity <= si::from_km_h(120.0)) {
    if (weight_per_vehicle <= si::from_ton(600.0)) return 3.0;
    return 4.0;
  } else if (current_max_velocity <= si::from_km_h(160.0)) {
    if (weight_per_vehicle <= si::from_ton(350.0)) return 3.0;
    if (weight_per_vehicle <= si::from_ton(500.0)) return 4.0;
    return 5.0;
  } else {  // over 160 km/h
    if (weight_per_vehicle <= si::from_ton(350.0)) return 4.0;
    if (weight_per_vehicle <= si::from_ton(500.0)) return 5.0;
    return 6.0;
  }
}

surcharge_factor::value_type surcharge_percentage_diesel(
    train_physics const& tp) {
  auto const diesel_count = utls::count_if(
      tp.units(), [](auto&& ctu) { return ctu.traction_unit_.is_diesel(); });

  auto const weight_per_vehicle = tp.weight() / diesel_count;

  return weight_per_vehicle > si::from_ton(400.0) ? 6.0 : 3.0;
}

surcharge_factor::value_type surcharge_percentage_ice(
    si::speed const current_max_velocity) {
  if (current_max_velocity <= si::from_km_h(120.0)) return 4.0;
  if (current_max_velocity <= si::from_km_h(200.0)) return 5.0;
  return 7.0;  // over 200 km/h
}

rs::surcharge_factor train_physics::surcharge_factor(
    si::speed const current_max_velocity) const {
  rs::surcharge_factor::value_type result = 0.0;

  if (is_freight()) {
    result = 5.0;
  } else if (has_diesel()) {
    result = surcharge_percentage_diesel(*this);
  } else if (has_electric()) {
    result = surcharge_percentage_electric(*this, current_max_velocity);
  } else if (train_category().is_ice()) {
    result = surcharge_percentage_ice(current_max_velocity);
  } else {
    result = 3.0;
  }

  return rs::surcharge_factor{1.0 + result / 100.0};
}

}  // namespace soro::rs
