#pragma once

#include <chrono>

#include "soro/utls/narrow.h"

namespace soro::utls {

struct cumulative_timer {
  void start() { start_ = std::chrono::high_resolution_clock::now(); }

  void stop() {
    ++count_;
    end_ = std::chrono::high_resolution_clock::now();
  }

  auto total_duration() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_);
  }

  auto avg_duration() const {
    return narrow<std::size_t>(total_duration().count()) / count_;
  }

  auto count() const { return count_; }

  std::size_t count_{0};
  std::chrono::high_resolution_clock::time_point start_;
  std::chrono::high_resolution_clock::time_point end_;
};

}  // namespace soro::utls