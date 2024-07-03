#include "soro/infrastructure/station/station_route.h"

#include <algorithm>
#include <iterator>
#include <optional>

#include "soro/base/soro_types.h"

#include "soro/utls/coroutine/recursive_generator.h"
#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/distance.h"
#include "soro/utls/std_wrapper/upper_bound.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/line.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/station/station_route_graph.h"

#include "soro/rolling_stock/stop_mode.h"

namespace soro::infra {

speed_limit::ptr route_node::get_speed_limit(
    infrastructure const& infra) const {
  utls::sassert(node_->is(type::SPEED_LIMIT), "no speed limit at node");

  if (alternate_spl_.has_value()) {
    return alternate_spl_.value();
  }

  auto const& spl = infra->graph_.get_element_data<speed_limit>(node_);

  return &spl;
}

station_route::idx station_route::size() const noexcept {
  return utls::narrow<idx>(nodes().size());
}

node::ptr station_route::nodes(station_route::idx const idx) const {
  return this->path_->nodes_[utls::narrow<soro::size_t>(idx)];
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

bool station_route::requires_etcs(lines const& lines) const {
  if (!path_->etcs_starts_.empty()) {
    auto const& etcs_start =
        nodes(path_->etcs_starts_.back())->element_->as<track_element>();
    auto const& line = lines.at(etcs_start.get_line());
    return !line.has_signalling(etcs_start.km());
  }

  return false;
}

bool station_route::requires_lzb(soro::infra::lines const& lines) const {
  if (!path_->lzb_starts_.empty()) {
    auto const& lzb_start =
        nodes(path_->lzb_starts_.back())->element_->as<track_element>();
    auto const& line = lines.at(lzb_start.get_line());
    return !line.has_signalling(lzb_start.km());
  }

  return false;
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

station_route::optional_idx station_route::get_halt_idx(
    rs::stop_mode const stop_mode) const {
  return is_passenger_stop_mode(stop_mode) ? passenger_halt_ : freight_halt_;
}

node::optional_ptr station_route::get_halt_node(
    rs::stop_mode const stop_mode) const {
  auto const opt_idx = get_halt_idx(stop_mode);
  return opt_idx.has_value() ? node::optional_ptr(nodes(*opt_idx))
                             : node::optional_ptr(std::nullopt);
}

station_route::optional_idx station_route::get_runtime_checkpoint_idx() const {
  return runtime_checkpoint_;
}

node::optional_ptr station_route::get_runtime_checkpoint_node() const {
  return runtime_checkpoint_.transform([&](auto&& idx) { return nodes(idx); });
}

struct iterating_indices {
  iterating_indices(station_route::idx const omit,
                    station_route::idx const espl,
                    station_route::idx const aspl)
      : omit_{omit}, espl_{espl}, aspl_{aspl} {}

  soro::size_t omit() const { return utls::narrow<soro::size_t>(omit_); }
  soro::size_t espl() const { return utls::narrow<soro::size_t>(espl_); }
  soro::size_t aspl() const { return utls::narrow<soro::size_t>(aspl_); }

  station_route::idx omit_;
  station_route::idx espl_;  // extra speed limits
  station_route::idx aspl_;  // alternative speed limits
};

iterating_indices fast_forward_indices(station_route const& sr,
                                       station_route::idx const ff_to) {
  iterating_indices result{0, 0, 0};

  if (ff_to == 0) return result;

  auto const omit_it = utls::upper_bound(sr.omitted_nodes_, ff_to - 1);
  result.omit_ = utls::distance<station_route::idx>(
      std::begin(sr.omitted_nodes_), omit_it);

  auto const idx_spl = [](auto&& idx, auto&& spl) { return idx < spl.idx_; };

  auto const espl_it =
      utls::upper_bound(sr.extra_speed_limits_, ff_to - 1, idx_spl);
  result.espl_ = utls::distance<station_route::idx>(
      std::begin(sr.extra_speed_limits_), espl_it);

  auto const aspl_it =
      utls::upper_bound(sr.alt_speed_limits_, ff_to - 1, idx_spl);
  result.aspl_ = utls::distance<station_route::idx>(
      std::begin(sr.alt_speed_limits_), aspl_it);

  return result;
}

iterating_indices fast_backward_indices(station_route const& sr,
                                        station_route::idx const fb_to) {
  using idx = station_route::idx;

  iterating_indices result(utls::narrow<idx>(sr.omitted_nodes_.size()) - 1,
                           utls::narrow<idx>(sr.extra_speed_limits_.size()) - 1,
                           utls::narrow<idx>(sr.alt_speed_limits_.size()) - 1);

  if (fb_to == sr.size() - 1) return result;

  auto const omit_it = std::upper_bound(
      std::rbegin(sr.omitted_nodes_), std::rend(sr.omitted_nodes_), fb_to + 1,
      [](auto&& idx, auto&& omitted) { return idx > omitted; });
  auto const odist = std::distance(std::rbegin(sr.omitted_nodes_), omit_it);
  auto const omit_idx = utls::narrow<idx>(sr.omitted_nodes_.size()) - odist - 1;
  result.omit_ = utls::narrow<idx>(omit_idx);

  auto const idx_spl = [](auto&& idx, auto&& spl) { return idx > spl.idx_; };
  auto const espl_it =
      std::upper_bound(std::rbegin(sr.extra_speed_limits_),
                       std::rend(sr.extra_speed_limits_), fb_to + 1, idx_spl);
  auto const edist =
      std::distance(std::rbegin(sr.extra_speed_limits_), espl_it);
  auto const espl_idx =
      utls::narrow<idx>(sr.extra_speed_limits_.size()) - edist - 1;
  result.espl_ = utls::narrow<idx>(espl_idx);

  auto const aspl_it =
      std::upper_bound(std::rbegin(sr.alt_speed_limits_),
                       std::rend(sr.alt_speed_limits_), fb_to + 1, idx_spl);
  auto const adist = std::distance(std::rbegin(sr.alt_speed_limits_), aspl_it);
  auto const aspl_idx =
      utls::narrow<idx>(sr.alt_speed_limits_.size()) - adist - 1;
  result.aspl_ = utls::narrow<idx>(aspl_idx);

  return result;
}

template <typename AdvanceFn, typename InitialIndicesFn>
utls::recursive_generator<route_node> from_to_impl(
    station_route const& sr, station_route::idx const from,
    station_route::idx const to, AdvanceFn const& advance,
    InitialIndicesFn const& get_initial_indices) {
  using idx = station_route::idx;

  route_node result;
  idx node_idx = from;

  iterating_indices ii = get_initial_indices(sr, from);

  for (; node_idx != to; node_idx = advance(node_idx)) {
    result.node_ = sr.nodes(node_idx);

    result.omitted_ = ii.omit_ < utls::narrow<idx>(sr.omitted_nodes_.size()) &&
                      ii.omit_ >= 0 && sr.omitted_nodes_[ii.omit()] == node_idx;

    if (result.omitted_) {
      ii.omit_ = advance(ii.omit_);
    }

    result.extra_spls_.clear();
    while (ii.espl_ < utls::narrow<idx>(sr.extra_speed_limits_.size()) &&
           ii.espl_ >= 0 &&
           sr.extra_speed_limits_[ii.espl()].idx_ == node_idx) {
      result.extra_spls_.push_back(&sr.extra_speed_limits_[ii.espl()].spl_);
      ii.espl_ = advance(ii.espl_);
    }

    result.alternate_spl_.reset();
    if (ii.aspl_ < utls::narrow<idx>(sr.alt_speed_limits_.size()) &&
        ii.aspl_ >= 0 && sr.alt_speed_limits_[ii.aspl()].idx_ == node_idx) {
      result.alternate_spl_.emplace(&sr.alt_speed_limits_[ii.aspl()].spl_);
      ii.aspl_ = advance(ii.aspl_);
    }

    co_yield result;
  }
}

utls::recursive_generator<route_node> station_route::from_to(
    station_route::idx const from, station_route::idx const to) const {

  if (from <= to) {
    if (from >= size() || to > size()) co_return;
    auto const increment = [](idx const i) -> idx { return i + 1; };
    co_yield from_to_impl(*this, from, to, increment, fast_forward_indices);
  } else {
    if (from >= size() || to < -1) co_return;
    auto const decrement = [](idx const i) -> idx { return i - 1; };
    co_yield from_to_impl(*this, from, to, decrement, fast_backward_indices);
  }
}

utls::recursive_generator<route_node> station_route::iterate() const {
  co_yield from_to(0, size());
}

utls::recursive_generator<route_node> station_route::backwards() const {
  co_yield from_to(size() - 1, -1);
}

utls::recursive_generator<route_node> station_route::from_fwd(
    station_route::idx from) const {
  co_yield from_to(from, size());
}

utls::recursive_generator<route_node> station_route::from_bwd(
    station_route::idx from) const {
  co_yield from_to(from, -1);
}

utls::recursive_generator<route_node> station_route::to_fwd(
    station_route::idx to) const {
  co_yield from_to(0, to);
}

utls::recursive_generator<route_node> station_route::to_bwd(
    station_route::idx to) const {
  co_yield from_to(size() - 1, to);
}

}  // namespace soro::infra