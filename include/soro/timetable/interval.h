#pragma once

#include "soro/base/time.h"

namespace soro::tt {

struct interval {
  bool valid() const { return start_ <= end_; }

  bool overlaps(interval const& other) const {
    return start_ <= other.end_ && other.start_ <= end_;
  }

  bool operator==(interval const& other) const = default;
  bool operator!=(interval const& other) const = default;

  absolute_time start_{absolute_time::min()};
  absolute_time end_{absolute_time::max()};
};

}  // namespace soro::tt
