#pragma once

#include "soro/rolling_stock/safety_systems.h"
#include "soro/rolling_stock/stop_mode.h"
#include "soro/rolling_stock/train_category.h"
#include "soro/rolling_stock/train_class.h"
#include "soro/rolling_stock/train_series.h"
#include "soro/rolling_stock/train_type.h"

namespace soro::rs {

using brake_weight_percentage = si::unitless;

using air_resistance_coefficient =  // s^2 / m^2
    decltype((std::declval<si::time>() * std::declval<si::time>()) /
             (std::declval<si::length>() * std::declval<si::length>()));
using bearing_friction_coefficient = si::unitless;

static constexpr auto carriage_mass_factor = si::unitless{1.06};
static constexpr auto default_wind = si::from_m_s(4.2);

using brake_position_key = soro::array<char, 3>;
using brake_position = soro::strong<uint8_t, struct _brake_position>;

using brake_type_key = soro::array<char, 3>;
using brake_type = soro::strong<uint8_t, struct _brake_type>;

using tilt_technology_key = soro::array<char, 1>;
using tilt_technology = soro::strong<uint8_t, struct _tilt_technology>;

using transportation_specialty_key = soro::array<char, 15>;
using transportation_specialty =
    soro::strong<uint16_t, struct _transport_specialty>;

using surcharge_factor = si::unitless;

struct configured_traction_unit {
  tractive_curve_t traction_curve() const;
  si::force tractive_force(si::speed const v) const;

  traction_unit traction_unit_;
  traction_unit::equipment::idx selected_{
      traction_unit::equipment::idx::invalid()};
};

struct train_physics {
#if !defined(SERIALIZE)
  train_physics() = default;
  // if we don't serialize we need a constructor since the members are private
  train_physics(soro::vector<configured_traction_unit> tus,
                si::weight const carriage_weight, si::length const length,
                si::speed const max_speed, soro::size_t const wagons,
                rs::LZB const lzb, rs::stop_mode const stop_mode,
                brake_position const brake_position,
                brake_type const brake_type,
                brake_weight_percentage const brake_weight_percentage,
                soro::optional<tilt_technology> const tilt_technology,
                air_resistance_coefficient const air_coefficient,
                bearing_friction_coefficient const bearing_coefficient,
                train_class train_class, train_category train_category,
                soro::vector<transportation_specialty> specialties);
#endif

  si::length length() const;
  si::length traction_vehicle_length() const;
  si::length carriage_length() const;

  si::weight weight() const;
  si::weight traction_vehicle_weight() const;
  si::weight carriage_weight() const;

  si::weight translatory_weight() const;
  si::weight translatory_vehicle_weight() const;
  si::weight translatory_carriage_weight() const;

  si::speed max_speed() const;
  si::speed max_speed(si::speed const speed_limit) const;

  si::speed approach_speed() const;

  si::force tractive_force(si::speed const v) const;
  si::force resistive_force(si::speed const v, si::slope const slope) const;

  si::accel acceleration(si::speed const current_speed,
                         si::slope const slope) const;
  si::accel natural_accel(si::speed const current_speed,
                          si::slope const slope) const;
  si::accel braking_deaccel(si::speed const infra_limit,
                            si::speed const bwp_limit,
                            si::length const brake_path_length) const;

  rs::surcharge_factor surcharge_factor(
      si::speed const current_max_velocity) const;

  rs::LZB lzb() const;
  bool has_lzb() const;

  rs::stop_mode stop_mode() const;

  bool is_freight() const;
  bool is_passenger() const;
  rs::train_type train_type() const;

  bool has_diesel() const;
  bool has_electric() const;

  rs::brake_type brake_type() const;

  rs::brake_position brake_position() const;

  rs::brake_weight_percentage percentage() const;

  soro::size_t vehicle_count() const;

  soro::vector<configured_traction_unit> const& units() const;

  soro::size_t wagons() const;

  rs::bearing_friction_coefficient bearing_friction() const;

  rs::air_resistance_coefficient air_resistance() const;

  soro::optional<rs::tilt_technology> const& tilt_technology() const;

  soro::vector<rs::transportation_specialty> const& specialties() const;

  rs::train_class const& train_class() const;

  rs::train_category const& train_category() const;

#if !defined(SERIALIZE)
  // if we don't need to serialize these members are private
  // since we do not want users to access them directly,
  // but use the provided methods instead
private:
#endif

  soro::vector<configured_traction_unit> traction_units_;
  si::weight carriage_weight_{si::weight::invalid()};
  si::length carriage_length_{si::length::invalid()};
  si::speed max_speed_{si::speed::invalid()};

  soro::size_t cars_{0};

  rs::LZB lzb_{rs::LZB::NO};
  rs::stop_mode stop_mode_{rs::stop_mode::passenger};

  rs::brake_position brake_position_{brake_position::invalid()};
  rs::brake_type brake_type_{brake_type::invalid()};
  brake_weight_percentage percentage_{brake_weight_percentage::invalid()};
  soro::optional<rs::tilt_technology> tilt_technology_;

  // only necessary for freight train cars
  air_resistance_coefficient air_resistance_;
  bearing_friction_coefficient bearing_friction_;

  // TODO(julian) we are making a copy of the train class here
  // maybe not the cleanest solution, but not a big deal
  rs::train_class train_class_;
  // TODO(julian) we are making a copy of the train category here
  // maybe not the cleanest solution, but not a big deal
  rs::train_category train_category_;

  soro::vector<rs::transportation_specialty> specialties_;
};

}  // namespace soro::rs
