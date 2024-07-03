#pragma once

#include "soro/infrastructure/critical_section.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/station/station_route.h"

namespace soro::infra {

struct infrastructure_t;
struct infrastructure;

struct sub_path {
  bool contains(station_route::idx const idx) const {
    return from_ <= idx && idx < to_;
  }

  station_route::ptr station_route_{nullptr};
  station_route::idx from_{station_route::invalid_idx()};
  station_route::idx to_{station_route::invalid_idx()};
};

struct interlocking_route {
  using id = strong<uint32_t, struct _interlocking_route_id>;
  using ids = soro::vector<id>;
  using ptr = soro::ptr<interlocking_route>;
  using sr_offset = uint8_t;

  static constexpr id invalid() { return std::numeric_limits<id>::max(); }
  static constexpr bool valid(id const id) noexcept { return id != invalid(); }

  bool static valid_end(type const t);
  type_set static valid_ends();

  bool operator==(interlocking_route const& o) const;

  bool follows(interlocking_route const& other,
               infrastructure const& infra) const;

  station_route::idx size(infrastructure const& infra) const;

  si::length length(infrastructure const& infra) const;

  bool contains(station_route::id, station_route::idx) const;

  station_route::id first_sr_id() const;
  station_route::id sr_id(sr_offset) const;
  station_route::id last_sr_id() const;

  station_route::ptr first_sr(infrastructure const&) const;
  station_route::ptr sr(sr_offset, infrastructure const&) const;
  station_route::ptr last_sr(infrastructure const&) const;

  node::ptr first_node(infrastructure const& infra) const;
  node::ptr last_node(infrastructure const& infra) const;

  element::ptr first_element(infrastructure const& infra) const;
  element::ptr last_element(infrastructure const& infra) const;

  bool starts_on_ms(infrastructure const&) const;
  bool ends_on_ms(infrastructure const&) const;

  bool starts_on_section(infrastructure const&) const;
  bool ends_on_section(infrastructure const&) const;

  critical_section::id get_start_critical_section(
      infrastructure const& infra) const;
  critical_section::id get_end_critical_section(
      infrastructure const& infra) const;

  utls::recursive_generator<route_node> from_to(station_route::id from_sr,
                                                station_route::idx from_idx,
                                                station_route::id to_sr,
                                                station_route::idx to_idx,
                                                infrastructure const&) const;

  utls::recursive_generator<route_node> from(station_route::id sr_id,
                                             station_route::idx from_idx,
                                             infrastructure const&) const;

  utls::recursive_generator<route_node> to(station_route::id sr_id,
                                           station_route::idx to_idx,
                                           infrastructure const&) const;

  utls::recursive_generator<route_node> iterate(infrastructure const&) const;
  utls::generator<sub_path> iterate_station_routes(
      infrastructure_t const&) const;

  id id_{invalid()};

  // defines the path of the interlocking route
  // from station_routes_.front()[start_offset_]
  // via station_routes[1:-2]
  // to station_routes_.back()[end_offset_ - 1]
  station_route::idx start_offset_{station_route::invalid_idx()};
  station_route::idx end_offset_{station_route::invalid_idx()};
  soro::vector<station_route::id> station_routes_;
};

// shorthand aliases
using ir_id = interlocking_route::id;

}  // namespace soro::infra
