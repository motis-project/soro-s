#include "soro/infrastructure/station/station_route.h"

#include "soro/infrastructure/station/station.h"
#include "soro/infrastructure/station/station_route_graph.h"

#include "soro/utls/coroutine/coro_map.h"
#include "soro/utls/sassert.h"

namespace soro::infra {

node::idx station_route::size() const noexcept {
  utls::sassert(
      this->path_->nodes_.size() < node::INVALID_IDX,
      "More nodes in a station route than node::idx is capable to hold.");

  return static_cast<node::idx>(nodes().size());
}

node::ptr station_route::nodes(node::idx const idx) const {
  return this->path_->nodes_[idx];
}

soro::vector<node::ptr> const& station_route::nodes() const {
  return this->path_->nodes_;
}

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

bool station_route::is_contained_route() const {
  return !starts_at_boundary(*this) && !ends_at_boundary(*this);
}

bool station_route::can_start_an_interlocking(
    station_route_graph const& srg) const {
  return !this->path_->main_signals_.empty() || srg.predeccesors_[id_].empty();
}

bool station_route::can_end_an_interlocking(
    station_route_graph const& srg) const {
  return !this->path_->main_signals_.empty() ||
         srg.successors_[this->id_].empty();
}

bool station_route::operator==(station_route const& o) const {
  return this->id_ == o.id_;
}

bool station_route::operator!=(station_route const& o) const {
  return !(*this == o);
}

node::optional_idx station_route::get_halt_idx(
    rs::FreightTrain const freight) const {
  return static_cast<bool>(freight) ? freight_halt_ : passenger_halt_;
}

node::optional_ptr station_route::get_halt_node(
    rs::FreightTrain const f) const {
  auto const opt_idx = get_halt_idx(f);
  return opt_idx.transform([&](node::idx const idx) { return nodes(idx); });
}

node::optional_ptr station_route::get_runtime_checkpoint_node() const {
  return runtime_checkpoint_.transform(
      [&](node::idx const idx) { return nodes(idx); });
}

std::pair<node::idx, node::idx> fast_forward_indices(station_route const& r,
                                                     node::idx const ff_to) {
  std::pair<node::idx, node::idx> indices{
      r.omitted_nodes_.empty() ? node::INVALID_IDX : 0,
      r.extra_speed_limits_.empty() ? node::INVALID_IDX : 0};

  if (ff_to == 0) {
    return indices;
  }

  if (!r.omitted_nodes_.empty()) {
    auto const omit_idx = utls::find_if_position(
        r.omitted_nodes_,
        [&ff_to](node::idx const omitted_idx) { return omitted_idx >= ff_to; });
    indices.first = static_cast<node::idx>(omit_idx);
  }

  if (r.extra_speed_limits_.empty()) {
    return indices;
  }

  node::idx spl_idx = 0;
  for (node::idx n_idx = 0; n_idx < ff_to; ++n_idx) {
    auto const& spl = r.extra_speed_limits_[spl_idx];
    if (spl.node_ == r.nodes(n_idx)) {
      ++spl_idx;
    }

    if (spl_idx == r.extra_speed_limits_.size()) {
      break;
    }
  }

  indices.second = spl_idx;

  return indices;
}

utls::recursive_generator<route_node> station_route::iterate() const {
  return from_to(0, size());
}

utls::recursive_generator<route_node> station_route::from_to(
    node::idx const from, node::idx const to) const {
  utls::sassert(from <= to, "To: {} is smaller than from: {}.", to, from);

  if (from >= size()) {
    co_return;
  }

  route_node result;
  node::idx node_idx = from;

  auto [omit_idx, spl_idx] = fast_forward_indices(*this, from);

  for (; node_idx < to; ++node_idx) {
    result.node_ = nodes(node_idx);

    result.omitted_ = omit_idx < omitted_nodes_.size() &&
                      omitted_nodes_[omit_idx] == node_idx;

    if (result.omitted_) {
      ++omit_idx;
    } else {
      bool const extra_spl =
          spl_idx < extra_speed_limits_.size() &&
          extra_speed_limits_[spl_idx].node_->id_ == result.node_->id_;

      if (extra_spl) {
        result.extra_spl_ = &extra_speed_limits_[spl_idx++];
      }
    }

    co_yield result;
  }
}

utls::recursive_generator<route_node> station_route::from(
    node::idx from) const {
  co_yield from_to(from, size());
}

utls::recursive_generator<route_node> station_route::to(node::idx to) const {
  co_yield from_to(0, to);
}

}  // namespace soro::infra