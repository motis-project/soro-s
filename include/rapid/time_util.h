#pragma once

#include <iosfwd>
#include <string>

namespace rapid {

struct unixtime {
  constexpr unixtime() = default;
  explicit constexpr unixtime(time_t t) : t_{t} {}  // NOLINT

  friend std::ostream& operator<<(std::ostream&, unixtime const&);

  unixtime& operator+=(unixtime const o) {
    t_ += o.t_;
    return *this;
  }
  unixtime& operator-=(unixtime const o) {
    t_ -= o.t_;
    return *this;
  }
  unixtime& operator*=(unixtime const o) {
    t_ -= o.t_;
    return *this;
  }
  unixtime& operator/=(unixtime const o) {
    t_ -= o.t_;
    return *this;
  }
  unixtime& operator++() {
    ++t_;
    return *this;
  }
  unixtime operator++(int) {
    auto copy = *this;
    ++t_;
    return copy;
  }
  unixtime& operator--() {
    --t_;
    return *this;
  }
  unixtime operator--(int) {
    auto copy = *this;
    --t_;
    return copy;
  }
  unixtime operator+(unixtime const o) const { return unixtime{t_ + o.t_}; }
  unixtime operator-(unixtime const o) const { return unixtime{t_ - o.t_}; }
  unixtime operator*(unixtime const o) const { return unixtime{t_ * o.t_}; }
  unixtime operator/(unixtime const o) const { return unixtime{t_ / o.t_}; }
  bool operator<(unixtime const o) const { return t_ < o.t_; }
  bool operator>(unixtime const o) const { return t_ > o.t_; }
  bool operator<=(unixtime const o) const { return t_ <= o.t_; }
  bool operator>=(unixtime const o) const { return t_ >= o.t_; }
  bool operator==(unixtime const o) const { return t_ == o.t_; }
  bool operator!=(unixtime const o) const { return t_ != o.t_; }

  operator time_t() const { return t_; }  // NOLINT

  time_t t_{0U};
};

std::string format_unix_time(time_t, char const* format = "%m/%d %H:%M:%OS");

}  // namespace rapid

namespace std {

template <>
struct numeric_limits<rapid::unixtime> {
  static constexpr rapid::unixtime max() {
    return rapid::unixtime{std::numeric_limits<time_t>::max()};
  }
  static constexpr rapid::unixtime min() {
    return rapid::unixtime{std::numeric_limits<time_t>::min()};
  }
};

template <>
struct is_integral<rapid::unixtime> : std::true_type {};

}  // namespace std