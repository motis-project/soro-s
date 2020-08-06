#pragma once

#include <ctime>
#include <array>
#include <iosfwd>
#include <vector>

#include "cista/containers/hash_map.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/unique_ptr.h"
#include "cista/reflection/comparable.h"

#include "soro/dpd.h"
#include "soro/speed_t.h"
#include "soro/unixtime.h"

namespace soro {

struct TimeGranularity {
  template <size_t Index>
  constexpr size_t get() {
    constexpr auto const s = std::array<size_t, 1>{size_t{6}};
    return s[Index];
  }
};

struct SpeedGranularity {
  template <size_t Index>
  constexpr size_t get() {
    constexpr auto const s = std::array<size_t, 1>{size_t{1}};
    return s[Index];
  }
};

struct TimeSpeedGranularity {
  template <size_t Index>
  constexpr size_t get() {
    constexpr auto const s = std::array<size_t, 2>{size_t{6}, size_t{1}};
    return s[Index];
  }
};

}  // namespace soro