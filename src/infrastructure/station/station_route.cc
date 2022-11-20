#include "soro/infrastructure/station/station_route.h"

#include "soro/infrastructure/station/station.h"
#include "soro/infrastructure/station/station_route_graph.h"

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

bool station_route::can_start_an_interlocking(
    station_route_graph const& srg) const {
  return !main_signals_.empty() || srg.predeccesors_[id_].empty();
}

bool station_route::can_end_an_interlocking(
    station_route_graph const& srg) const {
  return !this->main_signals_.empty() || srg.successors_[this->id_].empty();
}

utls::optional<node::idx> station_route::get_halt_idx(
    rs::FreightTrain const freight) const {
  return static_cast<bool>(freight) ? freight_halt_ : passenger_halt_;
}

utls::optional<infra::node_ptr> station_route::get_halt_node(
    rs::FreightTrain const f) const {
  auto const idx = get_halt_idx(f);
  return idx.transform([&](node::idx const idx) {
    return utls::optional<node::ptr>(nodes(idx));
  });
}

}  // namespace soro::infra