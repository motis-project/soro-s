#pragma once

#include "soro/si/units.h"

namespace soro::infra {

using kilometrage = si::length;

using mileage_dir_t = uint8_t;

enum class mileage_dir : mileage_dir_t { falling, rising, undirected };

constexpr bool is_falling(mileage_dir const dir) {
  return dir == mileage_dir::falling;
}

constexpr bool is_rising(mileage_dir const dir) {
  return dir == mileage_dir::rising;
}

constexpr bool is_undirected(mileage_dir const dir) {
  return dir == mileage_dir::undirected;
}

constexpr bool is_directed(mileage_dir const dir) {
  return dir != mileage_dir::undirected;
}

constexpr mileage_dir opposite(mileage_dir const m_dir) {
  return m_dir == mileage_dir::rising ? mileage_dir::falling
                                      : mileage_dir::rising;
}

constexpr std::string_view to_string(mileage_dir const m_dir) {
  return is_rising(m_dir)    ? "rising"
         : is_falling(m_dir) ? "falling"
                             : "undirected";
}

}  // namespace soro::infra