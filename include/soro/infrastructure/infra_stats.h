#pragma once

#include <array>
#include <functional>
#include <numeric>

#include "soro/infrastructure/graph/type.h"

namespace soro::infra {

struct infra_stats {
  using element_counts_t = std::array<std::size_t, type_count>;

  std::size_t& number(type const t) { return elements_[type_to_id(t)]; }
  std::size_t number(type const t) const { return elements_[type_to_id(t)]; }

  std::size_t total_elements() const {
    return std::accumulate(std::cbegin(elements_), std::cend(elements_),
                           std::size_t{0}, std::plus<>());
  }

  std::size_t sections_{};
  std::size_t stations_{};

  element_counts_t elements_{};
};

}  // namespace soro::infra