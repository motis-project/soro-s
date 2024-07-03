#pragma once

namespace soro::rs {

enum class stop_mode : bool { passenger, freight };

constexpr bool is_passenger_stop_mode(stop_mode const sm) {
  return sm == stop_mode::passenger;
}

constexpr bool is_freight_stop_mode(stop_mode const sm) {
  return sm == stop_mode::freight;
}

}  // namespace soro::rs
