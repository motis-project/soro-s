#pragma once

#include <limits>

#include "soro/base/soro_types.h"

#include "soro/infrastructure/graph/type.h"

namespace soro::infra {

struct element;
using element_ptr = soro::ptr<element>;

struct node {
  using id = uint32_t;
  using idx = uint16_t;
  using ptr = soro::ptr<node>;

  static constexpr auto INVALID = std::numeric_limits<id>::max();
  static constexpr auto INVALID_IDX = std::numeric_limits<idx>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }
  static constexpr bool valid_idx(id const idx) noexcept {
    return idx != INVALID_IDX;
  }

  bool is(type const t) const;

  ptr reverse_ahead() const;

  enum type type() const;

  id id_{INVALID};
  element_ptr element_{nullptr};

  // TODO(julian) maybe use std::optional here
  ptr next_node_{nullptr};
  ptr branch_node_{nullptr};

  // TODO(julian) the max size here should be 4 -> static vector
  soro::vector<ptr> reverse_edges_{};
};

}  // namespace soro::infra
