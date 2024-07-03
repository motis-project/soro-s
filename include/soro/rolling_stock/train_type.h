#pragma once

#include <iostream>

namespace soro::rs {

enum class train_type : bool { freight, passenger };

constexpr bool is_passenger(train_type const t) {
  return t == train_type::passenger;
}

constexpr bool is_freight(train_type const t) {
  return t == train_type::freight;
}

inline std::ostream& operator<<(std::ostream& out, train_type const tt) {
  if (tt == train_type::freight) {
    out << "freight";
  } else if (tt == train_type::passenger) {
    out << "passenger";
  }

  return out;
}

}  // namespace soro::rs