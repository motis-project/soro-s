#pragma once

#include "soro/simulation/kilometer_per_hour.h"
#include "soro/utls/unixtime.h"

namespace soro::simulation {

template <typename T>
struct granularity_base {
  explicit granularity_base(T const g) : g_{g} {}

  T g_;
};

template <typename T>
struct granularity;

template <>
struct granularity<kilometer_per_hour> : granularity_base<kilometer_per_hour> {
  granularity() : granularity_base(kilometer_per_hour{1}) {}
};

template <>
struct granularity<utls::unixtime> : granularity_base<utls::unixtime> {
  granularity() : granularity_base(utls::unixtime{6}) {}
};

template <>
struct granularity<utls::duration> : granularity_base<utls::duration> {
  granularity() : granularity_base(utls::duration{6}) {}
};

struct default_granularity {
  template <typename T>
  static constexpr T get() {
    return granularity<T>{}.g_;
  }
};

}  // namespace soro::simulation