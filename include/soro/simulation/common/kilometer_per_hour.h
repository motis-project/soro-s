#pragma once

#include <compare>
#include <string>

namespace soro::simulation {

struct kilometer_per_hour {
  using member_t = uint16_t;

  kilometer_per_hour() = default;
  template <
      typename T,
      std::enable_if_t<std::is_integral_v<T> && sizeof(T) >= 2, bool> = true>
  constexpr explicit kilometer_per_hour(T const val)
      : km_h_{static_cast<member_t>(val)} {}

  auto operator<=>(kilometer_per_hour const& o) const = default;

  auto operator+(kilometer_per_hour const& o) const {
    return kilometer_per_hour{static_cast<member_t>(km_h_ + o.km_h_)};
  }

  auto operator-(kilometer_per_hour const& o) const {
    return kilometer_per_hour{static_cast<member_t>(km_h_ - o.km_h_)};
  }

  auto operator/(kilometer_per_hour const& o) const {
    return kilometer_per_hour{static_cast<member_t>(km_h_ / o.km_h_)};
  }

  auto operator%(kilometer_per_hour const& o) const {
    return kilometer_per_hour{static_cast<member_t>(km_h_ % o.km_h_)};
  }

  auto& operator+=(kilometer_per_hour const& o) {
    km_h_ += o.km_h_;
    return *this;
  }

  auto& operator++() {
    ++km_h_;
    return *this;
  }

  explicit operator size_t() const { return size_t{km_h_}; }
  explicit operator float() const { return static_cast<float>(km_h_); }

  friend std::ostream& operator<<(std::ostream&, kilometer_per_hour const&);

  member_t km_h_{std::numeric_limits<member_t>::max()};
};

inline std::ostream& operator<<(std::ostream& os, kilometer_per_hour const& k) {
  return os << std::to_string(k.km_h_) << std::string("[km/h]");
}

constexpr kilometer_per_hour ZERO_KMH = kilometer_per_hour{0};

}  // namespace soro::simulation
