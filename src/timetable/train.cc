#include "soro/timetable/train.h"

#include <chrono>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <span>

#include "utl/enumerate.h"
#include "utl/verify.h"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/container/it_range.h"
#include "soro/utls/coroutine/coro_map.h"
#include "soro/utls/coroutine/recursive_generator.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/any_of.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/contains_if.h"
#include "soro/utls/std_wrapper/count_if.h"
#include "soro/utls/std_wrapper/find_if.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/path/length.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/rolling_stock/safety_systems.h"
#include "soro/rolling_stock/stop_mode.h"
#include "soro/rolling_stock/train_physics.h"
#include "soro/rolling_stock/train_type.h"

#include "soro/timetable/bitfield.h"
#include "soro/timetable/interval.h"
#include "soro/timetable/sequence_point.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

train_node::train_node(soro::infra::route_node const& rn,
                       sequence_point::optional_ptr const sp)
    : infra::route_node{rn}, sequence_point_{sp} {}

bool train_node::omitted() const {
  // omit halt nodes that have no sequence point attached
  auto const omit_halt = node_->is(type::HALT) && !sequence_point_.has_value();

  return omitted_ || omit_halt;
}

train::iterator::iterator(train const* train, interval const& interval,
                          bool const end)
    : train_{train},
      bitfield_it_(end ? train->service_days_.to(interval.end_)
                       : train->service_days_.from(interval.start_)),
      interval_{interval} {
  advance();
}

void train::iterator::advance() {
  while (bitfield_it_ != std::end(train_->service_days_) &&
         *bitfield_it_ <= interval_.end_ &&
         !interval_.overlaps(train_->event_interval(*bitfield_it_))) {
    ++bitfield_it_;
  }
}

train::iterator& train::iterator::operator++() {
  utls::expect(bitfield_it_ != std::end(train_->service_days_),
               "incrementing end bitfield iterator");
  ++bitfield_it_;
  advance();
  return *this;
}

train::iterator::value_type train::iterator::operator*() const {
  return absolute_time{*bitfield_it_};
}

utls::it_range<train::iterator> train::departures(
    interval const& interval) const {
  return utls::make_range(train::iterator{this, interval, false},
                          train::iterator{this, interval, true});
}

utls::it_range<train::iterator> train::departures() const {
  return departures(interval{});
}

rs::train_type train::type() const { return physics_.train_type(); }

stop_mode train::stop_mode() const { return this->physics_.stop_mode(); }

bool train::uses_freight() const {
  return this->stop_mode() == rs::stop_mode::freight;
}

bool train::has_lzb() const { return physics_.has_lzb(); };

rs::LZB train::lzb() const { return physics_.lzb(); }

si::length train::path_length(infrastructure const& infra) const {
  return get_path_length_from_elements(
      utls::coro_map(this->iterate(infra), [](auto&& rn) { return rn.node_; }));
}

absolute_time train::first_absolute_timestamp() const {
  return relative_to_absolute(service_days_.first_set_date(), start_time_);
}

absolute_time train::last_absolute_timestamp() const {
  return relative_to_absolute(service_days_.last_set_date(), end_time_);
}

interval train::event_interval(absolute_time const midnight) const {
  utls::sasserts([&]() {
    auto const floored = std::chrono::floor<days>(midnight);  // NOLINT
    utls::sassert(floored == midnight, "time point {} not midnight", midnight);
    utls::sassert(
        service_days_.at(
            sc::time_point_cast<bitfield::anchor_time::duration>(midnight)),
        "midnight value {} is not set in service day bitfield", midnight);
  });

  return {.start_ = relative_to_absolute(midnight, start_time_),
          .end_ = relative_to_absolute(midnight, end_time_)};
}

soro::size_t train::total_halts() const {
  return utls::count_if(sequence_points_,
                        [](auto&& sp) { return sp.is_halt(); });
}

soro::size_t train::total_stops() const {
  return utls::count_if(sequence_points_,
                        [](auto&& sp) { return sp.is_stop(); });
}

soro::size_t train::trip_count() const { return service_days_.count(); }

soro::size_t train::trip_count(interval const& interval) const {
  soro::size_t result = 0;

  for (auto const anchor_time : departures(interval)) {
    if (interval.overlaps(event_interval(anchor_time))) {
      ++result;
    }
  }

  return result;
}

soro::vector<train::trip> train::trips(trip::id const start_id) const {
  soro::vector<train::trip> result;
  result.reserve(trip_count());

  for (auto const [offset, anchor] :
       utl::enumerate<train::trip::id::value_t>(departures())) {
    result.emplace_back(start_id + offset, id_, anchor);
  }

  return result;
}

bool affected_by_train_type(train const& train, speed_limit const& spl) {
  return spl.train_type_.has_value() && *spl.train_type_ == train.type();
}

bool affected_by_train_series(train const& train, speed_limit const& spl) {
  return utls::any_of(spl.train_series_, [&](auto&& key) {
    return utls::contains_if(train.physics_.units(), [&](auto&& ctu) {
      return ctu.traction_unit_.key_.train_series_key_ == key;
    });
  });
}

bool affected_by_train_series_type(train const& train, speed_limit const& spl) {
  return utls::any_of(spl.train_series_types_, [&](auto&& type) {
    return utls::contains_if(train.physics_.units(), [&](auto&& ctu) {
      return ctu.traction_unit_.type_ == type;
    });
  });
}

bool affected_by_tilt(train const& train, speed_limit const& spl) {
  return train.physics_.tilt_technology().has_value() &&
         utls::contains(spl.tilt_technologies_,
                        *train.physics_.tilt_technology());
}

bool affected_by_transport_specialties(train const& train,
                                       speed_limit const& spl) {
  return utls::any_of(spl.transportation_specialties_, [&](auto&& specialty) {
    return utls::contains(train.physics_.specialties(), specialty);
  });
}

bool affected_by_specialty(train const& train, speed_limit const& spl) {
  utls::expect(spl.is_special(), "speed limit is not special");
  return affected_by_train_type(train, spl) ||
         affected_by_train_series(train, spl) ||
         affected_by_train_series_type(train, spl) ||
         affected_by_tilt(train, spl) ||
         affected_by_transport_specialties(train, spl);
}

bool affected_by_affects(train const& train, speed_limit const& spl) {
  return (!train.has_lzb() && spl.affects_conventional()) ||
         (train.has_lzb() && spl.affects_lzb());
}

bool train::affected_by(speed_limit const& spl) const {
  // TODO(julian) ignore speed limits with limit == 0 for now.
  if (!spl.ends_special() && spl.limit_.is_zero()) return false;

  return affected_by_affects(*this, spl) &&
         (!spl.is_special() || affected_by_specialty(*this, spl));
}

surcharge_factor train::surcharge_factor(
    si::speed const current_max_velocity) const {
  return physics_.surcharge_factor(current_max_velocity);
}

node::ptr train::get_start_node(infrastructure const& infra) const {
  utls::expect(!sequence_points_.empty(), "sequence points are empty");

  // TODO(julian) support trains with a transit as first sequence point
  utls::expect(break_in_ || sequence_points_.front().is_halt(),
               "non halt not supported");

  return break_in_ ? first_station_route(infra)->nodes().front()
                   : sequence_points_.front().get_node(infra);
}

node::ptr train::get_end_node(infrastructure const& infra) const {
  utls::expect(!sequence_points_.empty(), "sequence points are empty");

  // TODO(julian) support trains with a transit as first sequence point
  utls::expect(break_out_ || sequence_points_.back().is_halt(),
               "non halt not supported");

  return break_out_ ? last_station_route(infra)->nodes().back()
                    : sequence_points_.back().get_node(infra);
}

station_route::id train::first_station_route_id() const {
  return sequence_points_.front().station_route_;
}

station_route::id train::last_station_route_id() const {
  return sequence_points_.back().station_route_;
}

station_route::ptr train::first_station_route(
    infrastructure const& infra) const {
  return infra->station_routes_[first_station_route_id()];
}

infra::station_route::ptr train::last_station_route(
    infra::infrastructure const& infra) const {
  return infra->station_routes_[last_station_route_id()];
}

interlocking_route const& train::first_interlocking_route(
    infrastructure const& infra) const {
  utls::expect(!path_.empty(), "path is empty");
  utls::expect(path_.front() < infra->interlocking_.routes_.size(),
               "idx out of range");

  return infra->interlocking_.routes_[path_.front()];
}

infra::interlocking_route const& train::last_interlocking_route(
    infra::infrastructure const& infra) const {
  utls::expect(!path_.empty(), "path is empty");
  utls::expect(path_.back() < infra->interlocking_.routes_.size(),
               "idx out of range");

  return infra->interlocking_.routes_[path_.back()];
}

bool train::goes_through(station::id const s_id,
                         infrastructure const& infra) const {
  return utls::contains_if(sequence_points_, [&](auto&& sp) {
    return infra->station_routes_[sp.station_route_]->station_->id_ == s_id;
  });
}

std::span<sequence_point const> train::get_sequence_points(
    station_route::id sr_id) const {
  auto const first = utls::find_if(
      sequence_points_, [&](auto&& sp) { return sp.station_route_ == sr_id; });

  utl::verify(first != std::end(sequence_points_),
              "could not find sequence point for station route {} in train {}",
              sr_id, id_);

  auto const last =
      std::find_if(first, std::end(sequence_points_),
                   [&](auto&& sp) { return sp.station_route_ != sr_id; });

  return {first, last};
}

std::span<sequence_point const> train::get_sequence_points(
    station::id const s_id, infrastructure const& infra) const {

  auto const first = utls::find_if(sequence_points_, [&](auto&& sp) {
    return infra->station_routes_[sp.station_route_]->station_->id_ == s_id;
  });

  utl::verify(first != std::end(sequence_points_),
              "could not find sequence point for station {} in train {}", s_id,
              id_);

  auto const last =
      std::find_if(first, std::end(sequence_points_), [&](auto&& sp) {
        return infra->station_routes_[sp.station_route_]->station_->id_ != s_id;
      });

  return {first, last};
}

utls::recursive_generator<route_node> iterate_train(
    train const& t, infrastructure const& infra) {

  if (t.path_.size() == 1) {
    auto const from_sr_id =
        t.break_in_ ? t.first_interlocking_route(infra).station_routes_.front()
                    : t.sequence_points_.front().station_route_;
    auto const from_idx = t.break_in_
                              ? t.first_interlocking_route(infra).start_offset_
                              : t.sequence_points_.front().idx_;

    auto const to_sr_id =
        t.break_out_ ? t.last_interlocking_route(infra).station_routes_.back()
                     : t.sequence_points_.back().station_route_;
    station_route::idx const to_idx =
        t.break_out_ ? t.first_interlocking_route(infra).end_offset_
                     : t.sequence_points_.back().idx_ + 1;

    co_yield t.first_interlocking_route(infra).from_to(from_sr_id, from_idx,
                                                       to_sr_id, to_idx, infra);
    co_return;
  }

  if (t.break_in_) {
    co_yield t.first_interlocking_route(infra).iterate(infra);
  } else {
    auto const& first_sp = t.sequence_points_.front();

    utls::sassert(
        first_sp.is_halt(),
        "first sequence point in non-breaking train {} is not a halt.", t.id_);
    utls::sassert(first_sp.idx_ != station_route::invalid_idx(), "no sr index");

    co_yield t.first_interlocking_route(infra).from(first_sp.station_route_,
                                                    first_sp.idx_, infra);
  }

  for (auto i = 1U; i < t.path_.size() - 1; ++i) {
    auto const& ir = infra->interlocking_.routes_[t.path_[i]];
    // skip the first element, we already yielded it
    // two following interlocking routes share the first and last element
    co_yield ir.from(ir.first_sr_id(), ir.start_offset_ + 1, infra);
  }

  auto const last_ir = t.last_interlocking_route(infra);
  if (t.break_out_) {
    co_yield last_ir.from(last_ir.first_sr_id(), last_ir.start_offset_ + 1,
                          infra);
  } else {
    utls::sassert(t.sequence_points_.back().is_halt(),
                  "Last sequence point in non-breakout train {} is not a halt.",
                  t.id_);

    auto const sr_id = t.sequence_points_.back().station_route_;
    auto const node_idx = t.sequence_points_.back().idx_;

    utls::sassert(node_idx != station_route::invalid_idx(),
                  "All sequence points in a train must have a value. Last "
                  "sequence point in train {} does not.",
                  t.id_);

    co_yield last_ir.from_to(last_ir.first_sr_id(), last_ir.start_offset_ + 1,
                             sr_id, node_idx + 1, infra);
  }
}

utls::recursive_generator<train_node> train::iterate(
    infrastructure const& infra) const {
  uint32_t sp_idx = 0;
  for (auto&& rn : iterate_train(*this, infra)) {
    if (sp_idx < sequence_points_.size() &&
        sequence_points_[sp_idx].get_node(infra) == rn.node_) {

      co_yield train_node(
          rn, sequence_point::optional_ptr(&sequence_points_[sp_idx]));
      ++sp_idx;
    } else {
      co_yield train_node(rn, {});
    }
  }
}

}  // namespace soro::tt
