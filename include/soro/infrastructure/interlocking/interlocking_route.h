#pragma once

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/station/station_route.h"
#include "soro/rolling_stock/freight.h"

namespace soro::infra {

struct base_infrastructure;
struct infrastructure;

struct interlocking_route {
  using id = uint32_t;
  using ptr = soro::ptr<interlocking_route>;
  using sr_offset = uint8_t;

  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }

  type_set static valid_ends();

  //  si::length length() const;

  bool operator==(interlocking_route const& o) const;

  bool follows(interlocking_route const& other,
               infrastructure const& infra) const;

  std::vector<element_ptr> elements(infrastructure const&) const;

  station_route::id first_sr_id() const;
  station_route::id sr_id(sr_offset) const;
  station_route::id last_sr_id() const;

  station_route::ptr first_sr(infrastructure const&) const;
  station_route::ptr sr(sr_offset, infrastructure const&) const;
  station_route::ptr last_sr(infrastructure const&) const;

  node::ptr first_node(infrastructure const& infra) const;
  node::ptr last_node(infrastructure const& infra) const;

  bool starts_on_ms(infrastructure const&) const;
  bool ends_on_ms(infrastructure const&) const;

  utls::recursive_generator<node::id const> first_sr_nodes(
      skip_omitted, base_infrastructure const&) const;
  utls::recursive_generator<node::id const> last_sr_nodes(
      skip_omitted, base_infrastructure const&) const;

  utls::recursive_generator<route_node> iterate(skip_omitted,
                                                infrastructure const&) const;
  utls::recursive_generator<route_node> from_to(node::idx, node::idx,
                                                skip_omitted,
                                                infrastructure const&) const;
  utls::recursive_generator<route_node> to(node::idx, skip_omitted,
                                           infrastructure const&) const;
  utls::recursive_generator<route_node> from(node::idx, skip_omitted,
                                             infrastructure const&) const;

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
