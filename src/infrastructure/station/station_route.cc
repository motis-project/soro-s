#include "soro/infrastructure/station/station_route.h"

#include "soro/infrastructure/station/station.h"

#include "soro/utls/coroutine/collect.h"
#include "soro/utls/coroutine/coro_map.h"

namespace soro::infra {

bool starts_at_boundary(station_route const& sr) {
  return sr.nodes().front()->is(type::BORDER) ||
         sr.nodes().front()->is(type::TRACK_END);
}

bool ends_at_boundary(station_route const& sr) {
  return sr.nodes().back()->is(type::BORDER) ||
         sr.nodes().back()->is(type::TRACK_END);
}

bool station_route::electrified() const {
  return attributes_[attribute_index(ELECTRIFIED)];
}

bool station_route::is_through_route() const {
  return starts_at_boundary(*this) && ends_at_boundary(*this);
}

bool station_route::is_in_route() const {
  return starts_at_boundary(*this) && !ends_at_boundary(*this);
}

bool station_route::is_out_route() const {
  return !starts_at_boundary(*this) && ends_at_boundary(*this);
}

node::idx station_route::get_halt_idx(rs::FreightTrain const freight) const {
  return static_cast<bool>(freight) ? freight_halt_ : passenger_halt_;
}

infra::node_ptr station_route::get_halt_node(rs::FreightTrain const f) const {
  auto const idx = get_halt_idx(f);
  return idx == node::INVALID_IDX ? nullptr : nodes(idx);
}

}  // namespace soro::infra