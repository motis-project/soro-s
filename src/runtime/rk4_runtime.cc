#include "soro/runtime/rk4_runtime.h"

#include <vector>

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/any_of.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/timetable/train.h"

#include "soro/runtime/common/conversions.h"
#include "soro/runtime/common/event.h"
#include "soro/runtime/common/get_intervals.h"
#include "soro/runtime/common/get_stop_time.h"
#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/signal_time.h"
#include "soro/runtime/common/terminate.h"
#include "soro/runtime/common/timestamps.h"
#include "soro/runtime/common/use_surcharge.h"
#include "soro/runtime/strategy/shortest_travel_time.h"

namespace soro::runtime::rk4 {

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

void determine_events(std::vector<drive_event> const& events,
                      relative_time const start_time, si::time const current,
                      si::time const delta, si::time const stop_time,
                      rs::surcharge_factor const surcharge,
                      EventCB const& event_cb) {
  event e;

  for (auto const& event : events) {
    e.element_ = event.node_->element_;

    e.arrival_ =
        start_time + to_relative(current + (event.arrival_ * surcharge));
    e.departure_ = e.arrival_;

    if (event.arrival_ * surcharge == delta) {
      e.departure_ += to_relative(stop_time);
    }

    event_cb(e);
  }
}

rs::surcharge_factor surcharge_factor(train const& t, si::speed const limit,
                                      use_surcharge const use_surcharge) {
  if (use_surcharge == use_surcharge::yes) {
    return t.surcharge_factor(limit);
  } else {
    return rs::surcharge_factor{1.0};
  }
}

void calculate_interval(interval const& i, train const& t,
                        train::trip const& trip, runtime_state& current,
                        use_surcharge const use_surcharge,
                        signal_time const& signal_time,
                        EventCB const& event_cb) {
  auto delta = current.driver_.drive(
      current.train_state_, &current.train_safety_, i, t, trip, signal_time);

  auto const sc_factor = surcharge_factor(t, i.speed_limit(), use_surcharge);
  delta.time_ = delta.time_ * sc_factor;

  auto const stop_time = get_stop_time(
      i, t.start_time_ + to_relative(current.train_state_.time_ + delta.time_),
      trip, signal_time);

  determine_events(delta.events_, t.start_time_, current.train_state_.time_,
                   delta.time_, stop_time, sc_factor, event_cb);

  current.train_state_ += delta;
  current.train_state_.time_ += stop_time;

  utls::sassert(current.train_state_.speed_ <= i.target_speed(t.physics_),
                "no speeding, limit {}, but got {}", i.target_speed(t.physics_),
                current.train_state_.speed_);
}

relative_time runtime_calculation(train const& t, infrastructure const& infra,
                                  type_set const& record_types,
                                  use_surcharge const use_surcharge,
                                  EventCB const& event_cb,
                                  TerminateCb const& terminate_cb) {
  auto const intervals = get_intervals(t, record_types, infra);
  auto const start_time = t.start_time_;

  runtime_state current;
  current.train_state_.time_ = si::time::zero();
  current.train_state_.dist_ = si::length::zero();
  current.train_state_.speed_ = t.start_speed_;

  signal_time const signal_time;
  train::trip const trip(train::trip::id{0}, t.id_, ZERO<absolute_time>);
  utls::sassert(signal_time.time_ == ZERO<absolute_time>);

  for (auto const i : intervals) {
    calculate_interval(i, t, trip, current, use_surcharge, signal_time,
                       event_cb);

    auto const should_terminate = utls::any_of(
        i.records(), [&](auto&& e) { return terminate_cb(e.node_); });

    if (should_terminate) break;
  }

  //    determine_record_times(*interval, event_reached);

  return start_time + to_relative(current.train_state_.time_);
}

relative_time runtime_calculation(train const& t, infrastructure const& infra,
                                  type_set const& record_types,
                                  use_surcharge const use_surcharge,
                                  EventCB const& event_cb) {
  auto const terminate = [](auto&&) { return false; };

  return runtime_calculation(t, infra, record_types, use_surcharge, event_cb,
                             terminate);
}

timestamps runtime_calculation(train const& t, infrastructure const& infra,
                               infra::type_set const& record_types,
                               use_surcharge const use_surcharge) {
  timestamps ts;

  auto const event_cb = [&ts](event const& e) {
    utls::sassert(ts.times_.empty() || valid(e.arrival_),
                  "only the first timestamp can have invalid arrival");
    utls::sassert(e.departure_ >= e.arrival_);
    utls::sassert(ts.times_.empty() ||
                  e.arrival_ >= ts.times_.back().departure_);

    ts.type_indices_[e.element_->type()].emplace_back(
        utls::narrow<soro::size_t>(ts.times_.size()));

    ts.times_.push_back(e);
  };

  auto const terminate = [](node::ptr) { return false; };

  runtime_calculation(t, infra, record_types, use_surcharge, event_cb,
                      terminate);

  return ts;
}

}  // namespace soro::runtime::rk4