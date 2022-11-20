#pragma once

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/route.h"
#include "soro/infrastructure/station/station_route.h"
#include "soro/rolling_stock/freight.h"

namespace soro::infra {

struct infrastructure;

struct interlocking_route {
  using id = uint32_t;
  using ptr = soro::ptr<interlocking_route>;
  using sr_offset = uint8_t;

  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }

  type_set static valid_ends();

  //  si::length length() const;

  //  bool conflicts(ptr other) const;
  bool follows(ptr potential_previous, infrastructure const& infra) const;

  std::vector<element_ptr> elements(infrastructure const&) const;

  station_route::ptr first_sr(infrastructure const&) const;
  station_route::ptr sr(sr_offset, infrastructure const&) const;
  station_route::ptr last_sr(infrastructure const&) const;

  node::ptr first_node(infrastructure const& infra) const;
  node::ptr last_node(infrastructure const& infra) const;

  bool starts_on_ms(infrastructure const&) const;
  bool ends_on_ms(infrastructure const&) const;

  utls::generator<const node::ptr> iterate(infrastructure const&) const;

  //  node::idx get_halt_idx(rs::FreightTrain freight) const;
  //  node_ptr get_halt(rs::FreightTrain freight) const;

  //  node::idx size() const noexcept { return r_.size(); }
  //  auto const& nodes() const noexcept { return r_.nodes_; }
  //  auto const& nodes(node::idx const i) const noexcept { return r_.nodes_[i];
  //  } auto const& omitted_nodes() const noexcept { return r_.omitted_nodes_; }
  //  auto const& extra_spl() const noexcept { return r_.extra_speed_limits_; }
  //  auto entire(skip_omitted skip) const { return r_.entire(skip); }
  //  auto from_to(node::idx from, node::idx to, skip_omitted skip) const {
  //    return r_.from_to(from, to, skip);
  //  }

  node::ptr signal_eotd(infrastructure const& infra) const;
  soro::vector<node::ptr> route_eotds(infrastructure const& infra) const;

  soro::vector<node::ptr> passenger_halts(infrastructure const& infra) const;
  soro::vector<node::ptr> freight_halts(infrastructure const& infra) const;

  id id_{INVALID};

  // defines the path of the interlocking route
  // from station_routes_.front()[start_offset_]
  // via station_routes[1:-2]
  // to station_routes_.back()[end_offset_]
  node::idx start_offset_{node::INVALID_IDX};
  node::idx end_offset_{node::INVALID_IDX};
  soro::vector<station_route::id> station_routes_{};
};

// shorthand aliases
using ir_id = interlocking_route::id;
using ir_ptr = interlocking_route::ptr;

}  // namespace soro::infra
