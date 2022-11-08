#pragma once

#include <cstdint>
#include <limits>

#include "soro/base/soro_types.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/utls/coroutine/recursive_generator.h"

namespace soro::infra {

enum class skip_omitted : bool { OFF, ON };

struct route_node {
  node::idx node_idx_{node::INVALID_IDX};
  node::ptr node_{nullptr};
  std::optional<speed_limit::ptr> extra_spl_{std::nullopt};
  bool omitted_{false};
};

struct route {
  node::idx size() const noexcept;

  utls::recursive_generator<route_node> from_to(node::idx from, node::idx to,
                                                skip_omitted skip) const;
  utls::recursive_generator<route_node> entire(skip_omitted skip) const;

  soro::vector<node_ptr> nodes_;
  soro::vector<node::idx> omitted_nodes_;
  soro::vector<speed_limit> extra_speed_limits_;
};

}  // namespace soro::infra