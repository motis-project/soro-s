#pragma once

#include <compare>

namespace soro::utls {

template <typename Precision, typename... Tags>
struct strong_type {
  using precision_t = Precision;

  auto operator<=>(strong_type const& o) const = default;

  auto& operator+=(strong_type const& o) {
    val_ += o.val_;
    return *this;
  }

  friend auto operator+(strong_type& lhs, strong_type const& rhs) {
    lhs += rhs;
    return lhs;
  }

  auto& operator-=(strong_type const& o) {
    val_ -= o.val_;
    return *this;
  }

  friend auto operator-(strong_type& lhs, strong_type const& rhs) {
    lhs -= rhs;
    return lhs;
  }

  auto& operator*=(strong_type const& o) {
    val_ *= o.val_;
    return *this;
  }

  friend auto operator*(strong_type& lhs, strong_type const& rhs) {
    lhs *= rhs;
    return lhs;
  }

  auto& operator/=(strong_type const& o) {
    val_ /= o.val_;
    return *this;
  }

  friend auto operator/(strong_type& lhs, strong_type const& rhs) {
    lhs /= rhs;
    return lhs;
  }

  precision_t val_;
};

}  // namespace soro::utls