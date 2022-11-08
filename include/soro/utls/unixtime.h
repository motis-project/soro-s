#pragma once

#include <chrono>
#include <compare>
#include <ctime>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <string>

#include "date/date.h"

namespace soro::utls {

constexpr char const* const INPUT_FORMAT_STR = "%F %T";
constexpr char const* const OUTPUT_FORMAT_STR = "%F %H:%M:%OS";

struct duration {
  using member_t = time_t;

  static constexpr auto INVALID_VALUE = std::numeric_limits<member_t>::max();
  static const duration INVALID;
  static const duration ZERO;

  constexpr duration() = default;
  explicit constexpr duration(time_t d) : d_(d) {}

  auto operator<=>(duration const& d) const = default;

  explicit operator time_t() const { return d_; }

  auto& operator+=(duration const o) {
    d_ += o.d_;
    return *this;
  }

  auto& operator-=(duration const o) {
    d_ -= o.d_;
    return *this;
  }

  auto operator+(duration const o) const { return duration{d_ + o.d_}; }
  auto operator-(duration const o) const { return duration{d_ - o.d_}; }
  auto operator*(duration const o) const { return duration{d_ * o.d_}; }
  auto operator/(duration const o) const { return duration{d_ / o.d_}; }
  auto operator%(duration const o) const { return duration{d_ % o.d_}; }

  explicit operator size_t() const { return static_cast<size_t>(d_); }

  friend std::ostream& operator<<(std::ostream&, duration const&);

  time_t d_{INVALID_VALUE};
};

struct unixtime {
  using member_t = time_t;

  constexpr unixtime() = default;
  explicit constexpr unixtime(time_t t) : t_{t} {}

  unixtime(char const* const source, char const* format_str) {
    date::sys_seconds t;
    std::istringstream{source} >> date::parse(format_str, t);
    t_ = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                 t - date::sys_seconds(std::chrono::seconds{0}))
                                 .count());
  }

  unixtime(std::string const& source, char const* format_str)
      : unixtime{source.c_str(), format_str} {}

  friend std::ostream& operator<<(std::ostream&, unixtime const&);

  auto operator+(unixtime const& o) const { return unixtime{t_ + o.t_}; }
  auto operator-(unixtime const& o) const { return unixtime{t_ - o.t_}; }
  auto operator*(unixtime const& o) const { return unixtime{t_ * o.t_}; }
  auto operator/(unixtime const& o) const { return unixtime{t_ / o.t_}; }
  auto operator%(unixtime const& o) const { return unixtime{t_ % o.t_}; }

  auto operator+(duration const& o) const { return unixtime{t_ + o.d_}; }
  auto operator-(duration const& o) const { return unixtime{t_ - o.d_}; }

  auto& operator+=(unixtime const& o) {
    t_ += o.t_;
    return *this;
  }

  template <typename T>
  constexpr bool operator==(T const& o) const {
    if constexpr (std::is_same_v<T, unixtime>) {
      return t_ == o.t_;
    } else {
      return t_ == o;
    }
  }

  template <typename T>
  auto operator<=>(T const& o) const {
    if constexpr (std::is_same_v<T, unixtime>) {
      return t_ <=> o.t_;
    } else {
      return t_ <=> o;
    }
  }

  explicit operator time_t() const { return t_; }
  explicit operator float() const { return static_cast<float>(t_); }
  explicit operator size_t() const { return static_cast<size_t>(t_); }

  duration as_duration() const { return duration{t_}; }
  bool in_interval(unixtime start, unixtime end) const;

  member_t t_{std::numeric_limits<decltype(t_)>::max()};
};

std::string format_unix_time(unixtime, char const* format = OUTPUT_FORMAT_STR);

}  // namespace soro::utls

namespace std {

template <>
struct numeric_limits<soro::utls::unixtime> {
  static constexpr bool is_integer =
      std::is_integral_v<soro::utls::unixtime::member_t>;
  static constexpr bool is_signed =
      std::is_signed_v<soro::utls::unixtime::member_t>;

  static constexpr soro::utls::unixtime max() {
    return soro::utls::unixtime{
        std::numeric_limits<soro::utls::unixtime::member_t>::max()};
  }
  static constexpr soro::utls::unixtime min() {
    return soro::utls::unixtime{
        std::numeric_limits<soro::utls::unixtime::member_t>::min()};
  }
};

template <>
struct is_integral<soro::utls::unixtime> : std::true_type {};

template <>
struct is_integral<soro::utls::duration> : std::true_type {};

template <>
struct numeric_limits<soro::utls::duration> {
  static constexpr bool is_integer =
      std::is_integral_v<soro::utls::duration::member_t>;
  static constexpr bool is_signed =
      std::is_signed_v<soro::utls::duration::member_t>;

  static constexpr soro::utls::duration max() {
    return soro::utls::duration{
        std::numeric_limits<soro::utls::duration::member_t>::max()};
  }
  static constexpr soro::utls::duration min() {
    return soro::utls::duration{
        std::numeric_limits<soro::utls::duration::member_t>::min()};
  }
};

}  // namespace std

namespace soro::utls {

constexpr unixtime EPOCH = unixtime{0};
constexpr unixtime END_OF_TIME = std::numeric_limits<unixtime>::max();
constexpr unixtime INVALID_TIME = std::numeric_limits<unixtime>::max();

constexpr duration INVALID_DURATION = duration{duration::INVALID_VALUE};
constexpr duration ZERO_DURATION = duration{0};

inline namespace literals {

constexpr duration operator""_s(unsigned long long v) {
  return duration{static_cast<time_t>(v)};
}

constexpr duration operator""_m(unsigned long long v) {
  return duration{static_cast<time_t>(v * 60)};
}

constexpr duration operator""_h(unsigned long long v) {
  return duration{static_cast<time_t>(v * 60 * 60)};
}

}  // namespace literals

inline bool valid(unixtime const& ut) { return ut != INVALID_TIME; }
inline bool valid(duration const& d) { return d != INVALID_DURATION; }

}  // namespace soro::utls
