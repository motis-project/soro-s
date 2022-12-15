#pragma once

#include "soro/utls/container/id_iterator.h"

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/station/station_route.h"
#include "soro/rolling_stock/freight.h"

namespace soro::infra {

struct infrastructure_t;
struct infrastructure;

struct sub_path {
  bool contains(node::idx const idx) const { return from_ <= idx && idx < to_; }

  station_route::ptr station_route_{nullptr};
  node::idx from_{node::INVALID_IDX};
  node::idx to_{node::INVALID_IDX};
};

struct interlocking_route {
  using id = uint32_t;
  using ptr = soro::ptr<interlocking_route>;
  using sr_offset = uint8_t;

  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }

  type_set static valid_ends();

  bool operator==(interlocking_route const& o) const;

  bool follows(interlocking_route const& other,
               infrastructure const& infra) const;

  std::vector<element_ptr> elements(infrastructure const&) const;

  node::idx size(infrastructure const& infra) const;

  bool contains(station_route::id, node::idx) const;

  utls::it_range<utls::id_it_ptr<station_route>> station_routes(
      infrastructure const&) const;

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

  utls::recursive_generator<route_node> from_to(station_route::id, node::idx,
                                                station_route::id, node::idx,
                                                infrastructure const&) const;

  utls::recursive_generator<route_node> from(station_route::id, node::idx,
                                             infrastructure const&) const;
  utls::recursive_generator<route_node> to(station_route::id, node::idx,
                                           infrastructure const&) const;

  utls::recursive_generator<route_node> iterate(infrastructure const&) const;
  utls::generator<sub_path> iterate_station_routes(
      infrastructure_t const&) const;

  id id_{INVALID};

  // defines the path of the interlocking route
  // from station_routes_.front()[start_offset_]
  // via station_routes[1:-2]
  // to station_routes_.back()[end_offset_ - 1]
  node::idx start_offset_{node::INVALID_IDX};
  node::idx end_offset_{node::INVALID_IDX};
  soro::vector<station_route::id> station_routes_{};
};

// shorthand aliases
using ir_id = interlocking_route::id;
using ir_ptr = interlocking_route::ptr;

}  // namespace soro::infra
