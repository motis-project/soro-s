#pragma once

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/route.h"
#include "soro/infrastructure/station/station_route.h"
#include "soro/rolling_stock/freight.h"

namespace soro::infra {

struct interlocking_route {
  using id = uint32_t;
  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }
  using ptr = soro::ptr<interlocking_route>;

  type_set static valid_ends();

  si::length length() const;

  bool conflicts(ptr other) const;
  bool follows(ptr potential_previous) const;

  std::vector<element_ptr> elements() const;
  bool starts_on_ms() const;
  bool ends_on_ms() const;

  node::idx get_halt_idx(rs::FreightTrain freight) const;
  node_ptr get_halt(rs::FreightTrain freight) const;

  node::idx size() const noexcept { return r_.size(); }
  auto const& nodes() const noexcept { return r_.nodes_; }
  auto const& nodes(node::idx const i) const noexcept { return r_.nodes_[i]; }
  auto const& omitted_nodes() const noexcept { return r_.omitted_nodes_; }
  auto const& extra_spl() const noexcept { return r_.extra_speed_limits_; }
  auto entire(skip_omitted skip) const { return r_.entire(skip); }
  auto from_to(node::idx from, node::idx to, skip_omitted skip) const {
    return r_.from_to(from, to, skip);
  }

  id id_{INVALID};

  route r_;

  // the station routes the signal route is made out of
  soro::vector<station_route::ptr> station_routes_;

  node::idx signal_eotd_{node::INVALID_IDX};
  soro::vector<node::idx> route_eotds_;

  soro::vector<node::idx> passenger_halts_;
  soro::vector<node::idx> freight_halts_;
};

// shorthand aliases
using ir_id = interlocking_route::id;
using ir_ptr = interlocking_route::ptr;

}  // namespace soro::infra
