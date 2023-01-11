#pragma once

#include <map>

#include "utl/logging.h"

namespace soro::utls {

template <typename Enum>
struct statistics {
  using enum_t = Enum;
  using stat_texts = std::map<enum_t, std::string>;

  static_assert(static_cast<int>(enum_t::count) > 0,
                "Please ensure that there is an enum_t::count available");

  explicit statistics(std::map<enum_t, std::string> const& stat_text)
      : stat_texts_{stat_text} {
    for (auto i = 0; i < static_cast<int>(enum_t::count) - 1; ++i) {
      stats_.emplace(static_cast<enum_t>(i), 0);
    }
  }

  void inc(enum_t const stat) { ++stats_[stat]; }
  void dec(enum_t const stat) { --stats_[stat]; }

  void print() const {
    for (std::size_t i = 0; i < static_cast<std::size_t>(enum_t::count) - 1;
         ++i) {
      auto const k = static_cast<enum_t>(i);
      uLOG(utl::info) << stat_texts_.at(k) << " " << stats_.at(k);
    }
  }

  std::map<enum_t, int> stats_;
  std::map<enum_t, std::string> stat_texts_;
};

}  // namespace soro::utls
