#include "soro/runtime/euler_runtime.h"

#include <iterator>
#include <tuple>
#include <utility>

#include "utl/verify.h"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/any_of.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/train.h"

#include "soro/runtime/common/conversions.h"
#include "soro/runtime/common/event.h"
#include "soro/runtime/common/get_intervals.h"
#include "soro/runtime/common/get_stop_time.h"
#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/runtime_result.h"
#include "soro/runtime/common/terminate.h"
#include "soro/runtime/common/timestamps.h"
#include "soro/runtime/common/use_surcharge.h"
#include "soro/runtime/physics/euler/euler_physics.h"

namespace soro::runtime::euler {

using namespace soro::si;
using namespace soro::rs;
using namespace soro::utls;
using namespace soro::tt;
using namespace soro::infra;

struct phases {
  bool has_accel_phase() const { return !accel_results_.empty(); }
  bool has_coast_phase() const { return !coast_results_.empty(); }
  bool has_deaccel_phase() const { return !deaccel_results_.empty(); }

  runtime_result accel_;
  runtime_results accel_results_;

  runtime_result coast_;
  runtime_results coast_results_;

  runtime_result deaccel_;
  runtime_results deaccel_results_;
};

struct interval_times {
  times start_;
  times end_;
};

auto adjust_accel_and_deaccel(runtime_results const& accel_results,
                              length const delta_distance,
                              speed const next_speed_limit,
                              accel const deaccel) {
  // if acceleration distance and deacceleration distance is larger than
  // available distance don't accelerate to top speed, but stop before.

  auto accel_it = std::prev(std::cend(accel_results));

  runtime_results new_deaccel_results;
  while (true) {
    utl::verify(accel_it != std::cbegin(accel_results),
                "Undefined behaviour starting here.");

    --accel_it;

    if (accel_it->speed_ <= next_speed_limit) {
      new_deaccel_results.clear();
      break;
    }

    new_deaccel_results = brake(accel_it->speed_, next_speed_limit, deaccel);
    if (new_deaccel_results.back().dist_ + accel_it->dist_ <= delta_distance) {
      break;
    }
  }

  return std::pair(*accel_it, new_deaccel_results);
}

phases get_runtime_phases(si::speed current_speed, interval const& interval,
                          train_physics const& tp) {
  phases p;

  if (interval.length().is_zero()) return p;

  auto const current_limit = interval.speed_limit(tp);

  auto const deaccel =
      tp.braking_deaccel(interval.infra_limit(), interval.bwp_limit(),
                         interval.brake_path_length());

  // Acceleration phase
  if (current_speed < current_limit) {
    auto const initial_speed = current_speed;
    auto const target_speed = current_limit;
    p.accel_results_ = accelerate(tp, initial_speed, target_speed,
                                  interval.length(), interval.slope());
    p.accel_ = p.accel_results_.back();
    utls::sassert(p.accel_.speed_ <= current_limit, "no speeding");
    current_speed = p.accel_.speed_;
  }

  // Deacceleration phase, determined before coasting phase
  auto const target_speed = interval.target_speed(tp);
  if (current_speed > target_speed) {
    p.deaccel_results_ = brake(current_speed, target_speed, deaccel);
    utl::verify(!p.deaccel_results_.empty(), "Could not get any brake results");
    p.deaccel_ = p.deaccel_results_.back();
    utl::verify(p.deaccel_.time_ > time::zero(), "Wrong");

    utl::verify(p.deaccel_.dist_ <= interval.length(),
                "Not enough distance to deaccel");
  }

  // if acceleration distance and deacceleration distance is larger than
  // available distance don't accelerate to top speed, but stop before.
  if (p.accel_.dist_ + p.deaccel_.dist_ >= interval.length() &&
      p.has_accel_phase() && p.has_deaccel_phase()) {
    std::tie(p.accel_, p.deaccel_results_) = adjust_accel_and_deaccel(
        p.accel_results_, interval.length(), target_speed, deaccel);
    p.deaccel_ = p.deaccel_results_.empty() ? runtime_result{}
                                            : p.deaccel_results_.back();
    current_speed = p.accel_.speed_;
    utls::sassert(p.accel_.speed_ <= current_limit, "no speeding");
  }

  utl::verify(p.accel_.dist_ + p.deaccel_.dist_ <= interval.length() ||
                  !p.has_accel_phase() || !p.has_deaccel_phase(),
              "Not enough distance to accel and deaccel");

  // Coasting phase
  auto const coast_distance =
      interval.length() - p.accel_.dist_ - p.deaccel_.dist_;
  if (coast_distance > length::zero()) {
    auto const coast_speed = current_speed;

    p.coast_results_ = cruise(coast_speed, coast_distance);
    p.coast_ = p.coast_results_.back();
  }

  utl::verify(
      p.has_accel_phase() || p.has_coast_phase() || p.has_deaccel_phase(),
      "There must be at least one phase!");

  utls::ensures([&] {
    utls::ensure(p.accel_.dist_.is_valid(), "accel distance invalid");
    utls::ensure(p.accel_.speed_.is_valid(), "accel speed invalid");
    utls::ensure(p.accel_.time_.is_valid(), "accel time invalid");

    utls::ensure(p.coast_.dist_.is_valid(), "coast distance invalid");
    utls::ensure(p.coast_.speed_.is_valid(), "coast speed invalid");
    utls::ensure(p.coast_.time_.is_valid(), "coast time invalid");

    utls::ensure(p.deaccel_.dist_.is_valid(), "deaccel distance invalid");
    utls::ensure(p.deaccel_.speed_.is_valid(), "deaccel speed invalid");
    utls::ensure(p.deaccel_.time_.is_valid(), "deaccel time invalid");

    utls::ensure(current_limit >= p.accel_.speed_, "no speeding");
    utls::ensure(current_limit >= p.deaccel_.speed_, "no speeding");
    utls::ensure(current_limit >= p.coast_.speed_, "no speeding");

    utls::ensure(current_limit >= current_speed, "no speeding");
  });

  return p;
}

rs::surcharge_factor get_surcharge_factor(use_surcharge const use_surcharge,
                                          si::speed const max_speed,
                                          train const& t) {
  return use_surcharge == use_surcharge::yes ? t.surcharge_factor(max_speed)
                                             : rs::surcharge_factor{1.0};
}

template <typename EventReachedFn>
void determine_event_timestamps(interval const& interval, train const& t,
                                interval_times const& times,
                                phases const& phases,
                                use_surcharge const use_surcharge,
                                EventReachedFn const& event_reached) {

  auto const& get_event_time = [](length const delta_event_dist,
                                  struct phases const& p) -> si::time {
    if (!p.accel_results_.empty() && delta_event_dist <= p.accel_.dist_) {
      for (auto const& accel_result : p.accel_results_) {
        if (accel_result.dist_ >= delta_event_dist) {
          return accel_result.time_;
        }
      }
    } else if (!p.coast_results_.empty() &&
               delta_event_dist <= p.accel_.dist_ + p.coast_.dist_) {
      for (auto const& coast_result : p.coast_results_) {
        if (p.accel_.dist_ + coast_result.dist_ >= delta_event_dist) {
          return p.accel_.time_ + coast_result.time_;
        }
      }
    } else if (!p.deaccel_results_.empty() &&
               delta_event_dist <=
                   p.accel_.dist_ + p.coast_.dist_ + p.deaccel_.dist_) {
      for (auto const& deacc_result : p.deaccel_results_) {
        if (p.accel_.dist_ + p.coast_.dist_ + deacc_result.dist_ >=
            delta_event_dist) {
          return p.accel_.time_ + p.coast_.time_ + deacc_result.time_;
        }
      }
    }

    throw utl::fail("Could not find event time in phases!");
  };

  event e;

  for (auto const& record : interval.records()) {
    e.element_ = record.node_->element_;

    if (record.dist_ == interval.end_distance()) {
      e.arrival_ = times.end_.arrival_;
      e.departure_ = times.end_.departure_;

      event_reached(e);
      continue;
    }

    auto const sc_factor = get_surcharge_factor(
        use_surcharge, interval.speed_limit(t.physics_), t);
    auto const delta_record_distance = record.dist_ - interval.start_distance();
    auto const event_time =
        get_event_time(delta_record_distance, phases) * sc_factor;

    e.arrival_ = times.start_.departure_ + to_relative(event_time);
    e.departure_ = e.arrival_;

    event_reached(e);
  }
}

bool should_terminate(interval const& interval,
                      TerminateCb const& terminate_cb) {
  return utls::any_of(interval.records(),
                      [&](auto&& e) { return terminate_cb(e.node_); });
}

relative_time runtime_calculation(train const& t, infrastructure const& infra,
                                  type_set const& record_types,
                                  use_surcharge const use_surcharge,
                                  EventCB const& event_cb,
                                  TerminateCb const& terminate_cb) {
  auto const intervals = get_intervals(t, record_types, infra);

  auto current_speed = t.start_speed_;
  auto current_time = relative_time::zero();

  interval_times times;

  for (auto const& interval : intervals) {
    auto const p = get_runtime_phases(current_speed, interval, t.physics_);

    current_speed = p.accel_.speed_;
    current_speed = p.has_coast_phase() ? p.coast_.speed_ : current_speed;
    current_speed = p.has_deaccel_phase() ? p.deaccel_.speed_ : current_speed;

    utl::verify(current_speed <= interval.target_speed(),
                "going over speed limit is not allowed!");

    auto const max_speed = interval.speed_limit(t.physics_);
    auto const sc_factor = get_surcharge_factor(use_surcharge, max_speed, t);

    auto const travel_time =
        (p.accel_.time_ + p.coast_.time_ + p.deaccel_.time_) * sc_factor;

    times.start_.departure_ = t.start_time_ + current_time;
    times.end_.arrival_ = times.start_.departure_ + to_relative(travel_time);
    times.end_.departure_ = times.end_.arrival_;

    auto const stop_time =
        get_stop_time_on_timetable_stop(interval, times.end_.arrival_);
    times.end_.departure_ = times.end_.arrival_ + to_relative(stop_time);

    current_time += to_relative(travel_time + stop_time);

    determine_event_timestamps(interval, t, times, p, use_surcharge, event_cb);

    if (should_terminate(interval, terminate_cb)) break;
  }

  for (auto const& record : intervals.last_records()) {
    event e;

    e.arrival_ = times.end_.arrival_;
    e.departure_ = times.end_.departure_;

    e.element_ = record.node_->element_;

    event_cb(e);
  }

  return t.start_time_ + current_time;
}

relative_time runtime_calculation(train const& t, infrastructure const& infra,
                                  type_set const& record_types,
                                  use_surcharge const use_surcharge,
                                  EventCB const& event_cb) {
  return runtime_calculation(t, infra, record_types, use_surcharge, event_cb,
                             [](auto&&) { return false; });
}

timestamps runtime_calculation(train const& t, infrastructure const& infra,
                               infra::type_set const& record_types,
                               use_surcharge const use_surcharge) {
  timestamps ts;

  auto const event_cb = [&ts](event const& e) {
    utls::sassert(ts.times_.empty() || valid(e.arrival_),
                  "Only the first timestamp can have invalid arrival");
    utls::sassert(e.departure_ >= e.arrival_);
    utls::sassert(ts.times_.empty() ||
                  e.arrival_ >= ts.times_.back().departure_);

    ts.type_indices_[e.element_->type()].push_back(
        utls::narrow<soro::size_t>(ts.times_.size()));

    ts.times_.emplace_back(e);
  };

  auto const terminate = [](node::ptr) { return false; };

  runtime_calculation(t, infra, record_types, use_surcharge, event_cb,
                      terminate);

  return ts;
}

}  // namespace soro::runtime::euler
