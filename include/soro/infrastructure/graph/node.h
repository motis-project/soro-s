#pragma once

#include <limits>

#include "soro/utls/container/optional.h"

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

  using optional_id = soro::optional<id>;
  using optional_ptr = soro::optional<ptr>;
  using optional_idx = soro::optional<idx>;

  bool is(type const t) const;

  node::ptr reverse_ahead() const;

  enum type type() const;

  id id_{INVALID};
  element_ptr element_{nullptr};

  node::ptr next_node_{nullptr};
  node::ptr branch_node_{nullptr};

  soro::vector<node::ptr> reverse_edges_{};
};

}  // namespace soro::infra
