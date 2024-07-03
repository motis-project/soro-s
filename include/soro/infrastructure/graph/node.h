#pragma once

#include <limits>

#include "soro/utls/container/optional.h"

#include "soro/base/soro_types.h"

#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra {

struct element;
using element_ptr = soro::ptr<element>;

struct node {
  using id = cista::strong<uint32_t, struct _infra_node_id>;
  using ptr = soro::ptr<node>;

  static constexpr id invalid() { return std::numeric_limits<id>::max(); }

  using optional_id = soro::optional<id>;
  using optional_ptr = soro::optional<ptr>;

  id get_id() const;
  bool is(type const t) const;
  bool is_any(std::initializer_list<type> const types) const;

  node::ptr reverse_ahead() const;

  kilometrage km(node::ptr const neighbour) const;

  enum type type() const;
  std::string get_type_str() const;

  id id_{invalid()};
  element_ptr element_{nullptr};

  node::ptr next_{nullptr};
  node::ptr branch_{nullptr};

  soro::vector<node::ptr> reverse_edges_{};
};

}  // namespace soro::infra
