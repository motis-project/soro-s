#include "soro/runtime/common/get_intervals.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <optional>
#include <variant>

#include "cista/containers/variant.h"

#include "utl/erase_if.h"
#include "utl/overloaded.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/std_wrapper/is_sorted.h"
#include "soro/utls/std_wrapper/stable_sort.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/timetable/sequence_point.h"
#include "soro/timetable/train.h"

#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/speed_limit_manager.h"
#include "soro/runtime/physics/rk4/brake.h"

namespace soro::runtime {

using namespace soro::si;
using namespace soro::rs;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;
using namespace soro::runtime::rk4;

template <typename NodeFun>
void get_initial_break_in(train const& train, infrastructure const& infra,
                          NodeFun const& node_fun) {
  utls::expect(train.break_in_, "train not breaking in");

  auto const first_node = train.get_start_node(infra);
  auto const first_km = first_node->km(first_node);

  auto const last_node = first_node;
  for (auto n = last_node; n->km(last_node) != first_km; n = n->next_) {
    auto const should_terminate = node_fun(n);
    if (should_terminate) return;
  }
}

template <typename NodeFun>
auto get_initial_normal(train const& train, infrastructure const& infra,
                        NodeFun const& node_fun) {
  utls::expect(!train.break_in_, "train cannot be breaking in");
  utls::expect(!train.sequence_points_.empty(), "no sequence points in train");

  auto const& start = train.sequence_points_.front();

  utls::sassert(start.is_halt() && start.is_measurable());

  for (auto idx = start.idx_; idx > 0; --idx) {
    auto const node = train.first_station_route(infra)->nodes(idx);
    auto const should_terminate = node_fun(node);
    if (should_terminate) return;
  }

  auto curr_node = train.first_station_route(infra)->nodes().front();
  while (curr_node->reverse_ahead() != nullptr) {
    auto const should_terminate = node_fun(curr_node);
    if (should_terminate) return;

    curr_node = curr_node->reverse_ahead();
  }
}

template <typename NodeFun>
void get_initial(train const& train, infrastructure const& infra,
                 NodeFun const& node_fun) {
  train.break_in_ ? get_initial_break_in(train, infra, node_fun)
                  : get_initial_normal(train, infra, node_fun);
}

brake_path get_initial_brake_path(train const& train,
                                  infrastructure const& infra) {
  auto initial = infra->defaults_.brake_path_;

  auto const node_fun = [&](auto&& n) {
    if (!n->is(type::BRAKE_PATH)) return false;
    initial = infra->graph_.get_element_data<brake_path>(n);
    return true;
  };

  get_initial(train, infra, node_fun);

  return initial;
}

si::slope get_initial_slope(train const& train, infrastructure const& infra) {
  auto initial = si::slope::zero();

  auto const node_fun = [&](auto&& n) {
    if (!n->is(type::SLOPE)) return false;
    initial = infra->graph_.get_element_data<slope>(n);
    return true;
  };

  get_initial(train, infra, node_fun);

  return initial;
}

si::speed get_max_possible_speed_limit(si::speed const target_speed,
                                       si::accel const deaccel,
                                       si::length const length) {
  utls::sassert(length > si::length::zero(), "zero length");
  utls::sassert(deaccel < si::accel::zero(), "pos. deacceleration");

  si::time const t =
      target_speed / deaccel +
      ((target_speed.pow<2>() / deaccel.pow<2>()) + ((-2 * length) / deaccel))
          .sqrt();
  si::speed const new_speed = target_speed - deaccel * t;

  utls::sassert(!new_speed.is_zero(), "new speed should not be 0!");
  utls::sassert(new_speed >= target_speed,
                "new speed should be greater than target speed");

  return new_speed;
}

void fix_short_interval(interval_point& p1, interval_point const& p2,
                        train_physics const& tp) {
  interval const interval{&p1, &p2};

  auto const initial_speed = interval.speed_limit(tp);
  auto const target_speed = interval.target_speed(tp);

  // if we don't have to brake at all, we don't have to reduce the speed limit
  auto const braking_required =
      target_speed < initial_speed || interval.ends_on_stop();
  if (!braking_required) return;

  auto const deaccel =
      tp.braking_deaccel(interval.infra_limit(), interval.bwp_limit(),
                         interval.brake_path_length());

  auto const brake_result = brake(initial_speed, target_speed, deaccel);

  // if the interval is longer than the brake path we don't have to reduce
  // the speed limit
  if (interval.length() >= brake_result.dist_) return;

  auto const new_speed =
      get_max_possible_speed_limit(target_speed, deaccel, interval.length());

  utls::sassert(new_speed < initial_speed,
                "new speed limit should be smaller than old speed limit");

  p1.limit_ = new_speed;

  utls::sasserts([&interval, &deaccel, &new_speed, &target_speed] {
    auto const check_brake_result = brake(new_speed, target_speed, deaccel);
    utls::sassert(check_brake_result.dist_ <= interval.length());
  });
}

void adjust_signal_limit(interval_point& p1, interval_point const& p2,
                         train_physics const& tp) {
  interval const interval{&p1, &p2};

  auto const initial_speed = interval.signal_limit();
  auto const target_speed = interval.target_signal_speed();

  // if we don't have to brake at all, we don't have to reduce the speed limit
  auto const braking_required =
      interval.target_signal_speed() < initial_speed ||
      interval.ends_on_signal(type::MAIN_SIGNAL);
  if (!braking_required) return;

  auto const deaccel =
      tp.braking_deaccel(interval.infra_limit(), interval.bwp_limit(),
                         interval.brake_path_length());
  auto const brake_result = brake(initial_speed, target_speed, deaccel);

  // if the interval is longer than the brake path we don't have to reduce
  // the speed limit
  if (interval.length() >= brake_result.dist_) return;

  auto const new_speed =
      get_max_possible_speed_limit(target_speed, deaccel, interval.length());

  utls::sassert(new_speed < initial_speed,
                "new speed limit should be smaller than old speed limit");

  p1.signal_limit_ = new_speed;

  utls::sasserts([&interval, &deaccel, &new_speed, &target_speed] {
    auto const check_brake_result = brake(new_speed, target_speed, deaccel);
    utls::sassert(check_brake_result.dist_ <= interval.length());
  });
}

void fix_short_intervals(intervals& intervals, train_physics const& tp) {
  for (auto idx = intervals.p_.size() - 1; idx > 0; --idx) {
    // we would like to modify the speed limit of p1, non-const
    auto& p1 = intervals.p_[idx - 1];
    auto const& p2 = intervals.p_[idx];

    auto const zero_interval = p1.distance_ == p2.distance_;
    if (zero_interval) {
      // if we have a limit of length zero, we have to copy the limit
      // of p2 to p1, since we might have reduced the speed limit
      // to account for the brake path
      p1.limit_ = p2.limit_;
      p1.bwp_limit_ = p2.bwp_limit_;

      // only carry over the minimum of the signal limits,
      // since a main signal always has to have a signal limit of 0
      // but the following interval has signal_limit = speed_limit
      // and the preceding interval has to have signal_limit = 0
      p1.signal_limit_ = std::min(p1.signal_limit_, p2.signal_limit_);

      p1.sequence_point_ = p2.sequence_point_;

      continue;
    }

    fix_short_interval(p1, p2, tp);
    adjust_signal_limit(p1, p2, tp);
  }
}

void apply_brake_weight_percentage_limits(intervals& intervals,
                                          train const& train,
                                          infrastructure const& infra) {
  utls::expect(utls::all_of(intervals.p_,
                            [](auto&& p) { return p.avg_slope_.is_valid(); }),
               "avg slopes required");

  for (auto& p : intervals.p_) {
    // get the brake weight percentage induced speed limit
    auto const bwp_limit = infra->brake_tables_.get_max_speed(
        train.physics_.brake_type(), p.brake_path_, -p.avg_slope_,
        train.physics_.percentage());

    auto const current_limit = train.physics_.max_speed(p.limit_);

    utls::sassert(!current_limit.is_zero(), "got zero current speed limit");
    utls::sassert(!bwp_limit.is_zero(), "got zero bwp speed limit");

    p.bwp_limit_ = std::min(bwp_limit, current_limit);
  }
}

struct adjusted_node_list {
  struct border {};

  struct entry {
    si::length dist_;
    si::length original_dist_;
    cista::variant<si::slope, brake_path, speed_limit::ptr, sequence_point::ptr,
                   std::optional<signal>, border, std::monostate>
        data_;
  };

  template <typename Data>
  void add(si::length const dist, si::length const original_dist,
           Data const& data) {
    utls::sassert(dist.is_valid(), "got invalid distance");
    utls::sassert(original_dist.is_valid(), "got invalid distance");

    entries_.emplace_back(dist, original_dist, data);
  }

  void add(si::length const dist, node::ptr const node) {
    records_.emplace_back(node, dist);
  }

  soro::vector<entry> entries_;
  soro::vector<record> records_;
};

active_speed_limit entry_to_limit(adjusted_node_list::entry const& e) {
  utls::expect(holds_alternative<speed_limit::ptr>(e.data_));
  return {e.dist_, e.original_dist_, get<speed_limit::ptr>(e.data_)};
}

intervals adjusted_nodes_to_interval(adjusted_node_list const& adjusted,
                                     train const& train,
                                     infrastructure const& infra) {
  utls::expect(
      utls::is_sorted(adjusted.entries_,
                      [](auto&& e1, auto&& e2) { return e1.dist_ < e2.dist_; }),
      "needs to be sorted");

  intervals intervals;
  speed_limit_manager splm(train, infra);

  auto const initial_brake_path = get_initial_brake_path(train, infra);

  interval_point curr(
      si::length::zero(), splm.get_current_limit(),
      get_initial_slope(train, infra), initial_brake_path,
      infra->dictionaries_.brake_path_length_[initial_brake_path].value(),
      sequence_point::optional_ptr{}, {});

  auto from = std::begin(adjusted.records_);
  auto to = std::begin(adjusted.records_);

  bool border = false;

  for (auto const& entry : adjusted.entries_) {
    if (curr.distance_ != entry.dist_ || border) {
      to = std::find_if(from, std::end(adjusted.records_),
                        [&](auto&& e) { return e.dist_ > entry.dist_; });
      curr.records_ = soro::vector<record>(from, to);
      from = to;

      intervals.p_.push_back(curr);

      curr.sequence_point_.reset();
      curr.records_.clear();

      border = false;
    }

    curr.distance_ = entry.dist_;
    entry.data_.apply(utl::overloaded{
        [&](slope const slope) { curr.slope_ = slope; },
        [&](brake_path const path) {
          curr.brake_path_ = path;
          auto const length = infra->dictionaries_.brake_path_length_[path];
          utls::sassert(length.has_value(), "brake path length not found");
          curr.brake_path_length_ = length.value();
        },
        [&](sequence_point::ptr const p) { curr.sequence_point_.emplace(p); },
        [&](speed_limit::ptr const) {
          auto const should_update_limit = splm.add(entry_to_limit(entry));
          if (should_update_limit) curr.limit_ = splm.get_current_limit();
        },
        [&](std::optional<signal> const& s) {
          curr.last_signal_ = curr.next_signal_;
          curr.next_signal_ = s;
        },
        [&](adjusted_node_list::border) { border = true; }});

    // we have gone past the last signal, remove it
    if (curr.next_signal_.has_value() &&
        curr.distance_ > curr.next_signal_->dist_) {
      curr.next_signal_.reset();
    }
  }

  intervals.p_.push_back(curr);

  // we are copying the first valid signal to all intervals before the first
  // valid signal to ensure there is no interval at the start without a valid
  // signal
  auto const first_signal_it = utls::find_if(
      intervals.p_, [](auto&& p) { return p.next_signal_.has_value(); });
  if (first_signal_it != std::end(intervals.p_)) {
    std::for_each(std::begin(intervals.p_), first_signal_it, [&](auto&& p) {
      p.next_signal_ = first_signal_it->next_signal_;
    });
  }

  utls::ensure(
      utls::all_of(intervals.last_records(),
                   [&](auto&& record) {
                     return record.dist_ == intervals.p_.back().distance_;
                   }),
      "all last records must be on the same distance as the last point");

  utls::ensure(adjusted.records_.size() ==
                   utls::accumulate(intervals.p_, soro::size_t{0},
                                    [](auto&& acc, auto&& p) {
                                      return acc + p.records_.size();
                                    }),
               "all records must appear in the intervals");

  return intervals;
}

namespace detail {

si::length get_max_dist_to_cover(brake_path const bp,
                                 infrastructure const& infra) {
  auto const max_cover = si::from_m(2000.0);
  auto const length = infra->dictionaries_.brake_path_length_[bp];
  return length.has_value() ? std::min(*length, max_cover) : max_cover;
}

void set_avg_slopes(intervals& intervals, infrastructure const& infra) {
  for (auto idx = 0U; idx < intervals.p_.size() - 1; ++idx) {
    auto p1 = &intervals.p_[idx];
    auto const i = interval{p1, p1 + 1};

    auto const dist_to_cover = get_max_dist_to_cover(i.brake_path(), infra);

    // going backwards, starting at the current interval
    auto prev = i;
    auto covered = si::length::zero();
    si::slope avg_slope = si::slope::zero();

    // go until the distance is covered, or we run out of intervals
    while (covered < dist_to_cover && prev != std::rend(intervals)) {
      // covering either entire length of the interval,
      // or the remaining distance to go
      auto const covering = std::min(prev.length(), dist_to_cover - covered);
      avg_slope += si::as_m(covering) * prev.slope();
      covered += covering;

      --prev;
    }

    p1->avg_slope_ =
        !covered.is_zero() ? (avg_slope / si::as_m(covered)) : i.slope();
  }

  intervals.p_.back().avg_slope_ =
      intervals.p_[intervals.p_.size() - 2].avg_slope_;

  utls::ensure(utls::all_of(intervals.p_,
                            [](auto&& p) { return p.avg_slope_.is_valid(); }),
               "got invalid average slope");
}

void determine_minima(intervals& intervals, infrastructure const& infra) {
  for (auto idx = 0U; idx < intervals.p_.size() - 1; ++idx) {
    auto p1 = &intervals.p_[idx];
    auto next = interval{p1, p1 + 1};

    auto const dist_to_cover = get_max_dist_to_cover(p1->brake_path_, infra);

    auto min_avg_slope = next.avg_slope();
    auto covered = si::length::zero();

    while (covered < dist_to_cover && next != std::end(intervals)) {
      min_avg_slope = std::min(min_avg_slope, next.avg_slope());

      covered += next.length();
      ++next;
    }

    p1->avg_slope_ = min_avg_slope;
  }
}

}  // namespace detail

void set_average_slopes(intervals& intervals, infrastructure const& infra) {
  detail::set_avg_slopes(intervals, infra);
  detail::determine_minima(intervals, infra);
}

adjusted_node_list get_adjusted_node_list(train const& train,
                                          type_set const& record_types,
                                          infrastructure const& infra) {
  adjusted_node_list adjusted;

  auto current_dist = length::zero();
  auto prev_element = train.get_start_node(infra)->element_;

  // record the distance of the last signal (main or approach)
  // required for setting the next signal at every interval
  auto last_signal = length::zero();

  // record the distance of the last main signal
  // required for moving speed limits to the last main signal
  auto last_ms = length::invalid();

  for (auto const& tn : train.iterate(infra)) {
    current_dist += tn.node_->element_->get_distance(prev_element);
    prev_element = tn.node_->element_;

    if (tn.omitted()) continue;

    switch (tn.node_->type()) {
      case (type::BRAKE_PATH): {
        auto const current_brake_path =
            infra->graph_.get_element_data<brake_path>(tn.node_);
        adjusted.add(current_dist, current_dist, current_brake_path);
        break;
      }

      case (type::SLOPE): {
        auto const current_slope =
            infra->graph_.get_element_data<slope>(tn.node_);
        adjusted.add(current_dist, current_dist, current_slope);
        break;
      }

      case (type::MAIN_SIGNAL): {
        auto const sight_dist =
            std::max(si::length::zero(), current_dist - sight_distance);
        adjusted.add(sight_dist, current_dist, adjusted_node_list::border{});

        // pass last_signal to dist, as this is when this main signal
        // becomes the next_signal_
        // current_dist is the original dist of the main signal
        adjusted.add(  // NOLINT
            last_signal, current_dist,
            std::optional<signal>{signal{tn.node_->element_->get_id(),
                                         type::MAIN_SIGNAL, current_dist}});
        adjusted.add(current_dist, current_dist, adjusted_node_list::border{});

        last_ms = current_dist;
        last_signal = current_dist;

        break;
      }

      case (type::APPROACH_SIGNAL): {
        // pass last_signal to dist, as this is when this approach signal
        // becomes the next_signal_
        // current_dist is the original dist of the approach signal
        adjusted.add(  // NOLINT
            last_signal, current_dist,
            std::optional<signal>{signal{tn.node_->element_->get_id(),
                                         type::APPROACH_SIGNAL, current_dist}});
        adjusted.add(current_dist, current_dist, adjusted_node_list::border{});

        last_signal = current_dist;
        break;
      }

      case (type::TRACK_END): {
        adjusted.add(current_dist, current_dist, std::monostate{});
        break;
      }

      case (type::SPEED_LIMIT): {
        auto const spl = tn.get_speed_limit(infra);

        // TODO(julian) support speed limits with speed zero
        if (!spl->ends_special() && spl->limit_.is_zero()) break;

        // train is not affected by this speed limit, ignore
        if (!train.affected_by(*spl)) break;

        // we can ignore speed limits with from_last_signal when we have not
        // encountered a main signal yet
        if (spl->from_last_signal() && !last_ms.is_valid()) break;

        auto const dist = spl->from_last_signal() ? last_ms : current_dist;
        adjusted.add(dist, current_dist, spl);
        utls::sassert(!spl->length_.is_valid() || spl->length_.is_infinity(),
                      "length speed limit from infra");
        break;
      }

      default: {
        break;
      }
    }

    for (auto const& spl : tn.extra_spls_) {
      // TODO(julian) support speed limits with speed zero
      auto const is_zero = !spl->ends_special() && spl->limit_.is_zero();

      // we can ignore speed limits with from_last_signal when we have not
      // encountered a main signal yet
      auto const no_last_ms = spl->from_last_signal() && !last_ms.is_valid();

      // train is not affected by the speed limit, ignore
      auto const is_affected = train.affected_by(*spl);

      if (!is_zero && !no_last_ms && is_affected) {
        auto const dist = spl->from_last_signal() ? last_ms : current_dist;

        adjusted.add(dist, current_dist, spl);

        if (spl->length_.is_valid() && spl->length_ < si::length::max()) {
          auto const end_dist = dist + spl->length_;
          auto const original_dist = dist;
          adjusted.add(end_dist, original_dist, spl);
        }
      }
    }

    if (tn.sequence_point_.has_value() && (*tn.sequence_point_)->is_stop()) {
      adjusted.add(current_dist, current_dist, *tn.sequence_point_);
    }

    auto const is_halt =
        !tn.node_->is(type::HALT) ||
        (tn.sequence_point_.has_value() && (*tn.sequence_point_)->is_halt());
    if (record_types.contains(tn.node_->type()) && is_halt) {
      adjusted.add(current_dist, tn.node_);
    }
  }

  adjusted.add(current_dist, current_dist, adjusted_node_list::border{});
  adjusted.add(last_signal, last_signal, std::optional<signal>{std::nullopt});

  utls::stable_sort(adjusted.entries_,
                    [](auto&& e1, auto&& e2) { return e1.dist_ < e2.dist_; });

  return adjusted;
}

bool set_approach_limits(train const& train, soro::size_t const approach_idx,
                         soro::size_t const main_idx, intervals& intervals) {
  auto const approach_speed = train.physics_.approach_speed();

  si::length approach_speed_reached = intervals.p_[approach_idx].distance_;

  interval i{&intervals.p_[approach_idx], &intervals.p_[approach_idx + 1]};
  auto current_speed = i.speed_limit(train.physics_);

  for (; current_speed > approach_speed; ++i) {
    if (i.length().is_zero()) continue;

    auto const deaccel = train.physics_.braking_deaccel(
        i.infra_limit(), i.bwp_limit(), i.brake_path_length());

    auto const result = brake_over_distance(current_speed, deaccel, i.length());

    if (result.speed_ <= approach_speed) {
      auto const initial_speed = current_speed;
      auto const target_speed = approach_speed;
      auto const result2 = brake(initial_speed, target_speed, deaccel);
      approach_speed_reached += result2.dist_;
      current_speed = result2.speed_;
    } else {
      approach_speed_reached += i.length();
      current_speed = result.speed_;
    }
  }

  auto const it = std::find_if(
      std::begin(intervals.p_) + approach_idx, std::end(intervals.p_),
      [&](auto&& p) { return p.distance_ >= approach_speed_reached; });

  utls::sassert(it != std::begin(intervals.p_));

  auto const new_interval_required = it->distance_ != approach_speed_reached;

  auto const approach_start =
      new_interval_required ? intervals.p_.insert(it, *(it - 1)) : it;
  approach_start->distance_ = approach_speed_reached;
  // don't use it anymore, since we have inserted a new interval point
  // when new_interval_required is true

  utls::sassert(std::distance(approach_start,
                              std::begin(intervals.p_) + main_idx + 1) > 0,
                "no intervals after approach signal");

  // set the signal limit for the approach phase intervals
  // i.e. from reaching the approach speed (determined aboved)
  // until (but not including) the next main signal
  auto const to =
      std::begin(intervals.p_) + main_idx + (new_interval_required ? 1 : 0);
  std::for_each(approach_start, to,
                [&](auto&& p) { p.signal_limit_ = approach_speed; });

  if (new_interval_required) {
    utls::sassert(approach_start != std::begin(intervals.p_));

    utl::erase_if((approach_start - 1)->records_, [&](auto&& r) {
      return r.dist_ > approach_start->distance_;
    });

    utl::erase_if(approach_start->records_, [&](auto&& r) {
      return r.dist_ <= approach_start->distance_;
    });
  }

  return new_interval_required;
}

void set_signal_limits(intervals& intervals, train const& train) {
  soro::optional<soro::size_t> approach_idx;
  soro::optional<soro::size_t> main_idx;

  for (auto idx = 0U; idx < intervals.p_.size() - 1; ++idx) {
    // we would like to modify the signal speed limit of p1, non-const
    auto& p1 = intervals.p_[idx];
    auto const& p2 = intervals.p_[idx + 1];
    interval const i{&p1, &p2};

    // initialize every signal limit as the general speed limit
    // i.e. the neutral element wrt to the minimum of all limits
    p1.signal_limit_ = i.speed_limit();

    // TODO(julian) remove, just to test
    continue;

    if (p1.has_signal(type::MAIN_SIGNAL)) {
      p1.signal_limit_ = si::speed::zero();
      main_idx.emplace(idx);
    }

    if (p1.has_signal(type::APPROACH_SIGNAL)) approach_idx.emplace(idx);

    auto const in_approach = approach_idx.has_value() && main_idx.has_value() &&
                             approach_idx < main_idx;

    // if we are not approaching a main signal, signal_limit_ == speed_limit_
    if (!in_approach) continue;

    auto const new_interval_required =
        set_approach_limits(train, *approach_idx, *main_idx, intervals);

    // increment idx, since we added a new interval point
    if (new_interval_required) ++idx;

    // reset the approach signal and main signal idx, since we have inserted
    // the approach interval for this pair already
    approach_idx.reset();
    main_idx.reset();

    // don't do anything else with the interval after calling
    // insert_approach_intervals, since it modifies the intervals vector and the
    // interval is not valid anymore.
    // construct a new one!
  }

  intervals.p_.back().signal_limit_ =
      std::min(intervals.p_.back().limit_, intervals.p_.back().bwp_limit_);
}

intervals get_intervals(train const& train, type_set const& record_types,
                        infrastructure const& infra) {
  auto const adjusted = get_adjusted_node_list(train, record_types, infra);

  auto intervals = adjusted_nodes_to_interval(adjusted, train, infra);

  set_average_slopes(intervals, infra);

  apply_brake_weight_percentage_limits(intervals, train, infra);

  set_signal_limits(intervals, train);

  fix_short_intervals(intervals, train.physics_);

  utls::ensure(utls::is_sorted(intervals.p_, [](auto&& p1, auto&& p2) {
    return p1.distance_ < p2.distance_;
  }));

  return intervals;
}

}  // namespace soro::runtime
