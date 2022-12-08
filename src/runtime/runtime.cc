#include "soro/runtime/runtime.h"

#include "utl/logging.h"

#include "soro/runtime/interval.h"

#include "soro/utls/algo/slice.h"

namespace soro::runtime {

using namespace soro::si;
using namespace soro::utls;
using namespace soro::tt;
using namespace soro::infra;

const type_set BORDER_TYPES({type::HALT, type::MAIN_SIGNAL,
                             type::APPROACH_SIGNAL, type::SPEED_LIMIT});

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

auto adjust_accel_and_deaccel(rs::train_physics const& tv,
                              runtime_results const& accel_results,
                              length const delta_distance,
                              speed const next_speed_limit) {
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

    new_deaccel_results = brake(tv, accel_it->speed_, next_speed_limit);
    if (new_deaccel_results.back().distance_ + accel_it->distance_ <=
        delta_distance) {
      break;
    }
  }

  return std::pair(*accel_it, new_deaccel_results);
}

phases get_runtime_phases(runtime_result current, interval const& prev_interval,
                          interval const& interval,
                          rs::train_physics const& tp) {
  auto const interval_length = interval.distance_ - prev_interval.distance_;

  auto const current_max_speed = std::min(interval.limit_left_, tp.max_speed());

  phases p;

  // Acceleration phase
  if (current.speed_ < current_max_speed) {
    p.accel_results_ =
        accelerate(tp, current.speed_, current_max_speed, interval_length);
    p.accel_ = p.accel_results_.back();

    if (p.accel_.speed_ > current_max_speed) {
      p.accel_.speed_ = current_max_speed;
      utl::verify(
          p.accel_.speed_ - current_max_speed <= current_max_speed * 0.01F,
          "Accel speed calculation error more than 1%");
    }

    current.speed_ = p.accel_.speed_;
  }

  // Deacceleration phase, determined before coasting phase
  auto const target_speed = interval.target_speed(tp);
  if (current.speed_ > target_speed) {
    p.deaccel_results_ = brake(tp, current.speed_, target_speed);
    utl::verify(!p.deaccel_results_.empty(), "Could not get any brake results");
    p.deaccel_ = p.deaccel_results_.back();
    utl::verify(p.deaccel_.time_ > ZERO<time>, "Wrong");

    utl::verify(p.deaccel_.distance_ <= interval_length,
                "Not enough distance to deaccel");
  }

  // if acceleration distance and deacceleration distance is larger than
  // available distance don't accelerate to top speed, but stop before.
  if (p.accel_.distance_ + p.deaccel_.distance_ >= interval_length &&
      p.has_accel_phase() && p.has_deaccel_phase()) {
    std::tie(p.accel_, p.deaccel_results_) = adjust_accel_and_deaccel(
        tp, p.accel_results_, interval_length, target_speed);
    p.deaccel_ = p.deaccel_results_.empty() ? runtime_result{}
                                            : p.deaccel_results_.back();
    current.speed_ = p.accel_.speed_;
  }

  utl::verify(p.accel_.distance_ + p.deaccel_.distance_ <= interval_length ||
                  !p.has_accel_phase() || !p.has_deaccel_phase(),
              "Not enough distance to accel and deaccel");

  // Coasting phase
  auto const coast_distance =
      interval_length - p.accel_.distance_ - p.deaccel_.distance_;
  if (coast_distance > ZERO<length>) {
    auto const coast_speed = current.speed_;

    p.coast_results_ = coast(coast_speed, coast_distance);
    p.coast_ = p.coast_results_.back();
  }

  utl::verify(
      p.has_accel_phase() || p.has_coast_phase() || p.has_deaccel_phase(),
      "There must be at least one phase!");

  return p;
}

relative_time si_to_relative_time(si::time const t) {
  return relative_time(
      static_cast<relative_time::rep>(std::floor(si::as_s(t))));
}

si::time relative_to_si_time(relative_time const t) {
  return si::from_s(sc::duration_cast<seconds>(t).count());
}

template <typename EventReachedFn>
void determine_event_timestamps(interval const& prev_interval,
                                interval const& interval,
                                relative_time const start_arrival,
                                relative_time const end_arrival,
                                relative_time const end_departure,
                                phases const& phases,
                                EventReachedFn const& event_reached) {

  auto const& get_event_time = [](length const delta_event_dist,
                                  struct phases const& p) {
    if (!p.accel_results_.empty() && delta_event_dist <= p.accel_.distance_) {
      for (auto const& accel_result : p.accel_results_) {
        if (accel_result.distance_ >= delta_event_dist) {
          return accel_result.time_;
        }
      }
    } else if (!p.coast_results_.empty() &&
               delta_event_dist <= p.accel_.distance_ + p.coast_.distance_) {
      for (auto const& coast_result : p.coast_results_) {
        if (p.accel_.distance_ + coast_result.distance_ >= delta_event_dist) {
          return p.accel_.time_ + coast_result.time_;
        }
      }
    } else if (!p.deaccel_results_.empty() &&
               delta_event_dist <= p.accel_.distance_ + p.coast_.distance_ +
                                       p.deaccel_.distance_) {
      for (auto const& deacc_result : p.deaccel_results_) {
        if (p.accel_.distance_ + p.coast_.distance_ + deacc_result.distance_ >=
            delta_event_dist) {
          return p.accel_.time_ + p.coast_.time_ + deacc_result.time_;
        }
      }
    }

    throw utl::fail("Could not find event time in phases!");
  };

  for (auto const& event : interval.events_) {
    if (event.distance_ == interval.distance_) {
      event_reached(end_arrival, end_departure, event.node_->element_);
      continue;
    }

    length const delta_event_distance =
        event.distance_ - prev_interval.distance_;
    auto const event_time = get_event_time(delta_event_distance, phases);
    auto const time_stamp = start_arrival + si_to_relative_time(event_time);

    event_reached(time_stamp, time_stamp, event.node_->element_);
  }
}

template <typename EventReachedFn>
void runtime_calculation(train const& tr, infrastructure const& infra,
                         EventReachedFn const& event_reached,
                         type_set const& allowed_events) {
  utls::sassert(!tr.break_in_ && !tr.break_out_, "Not supported.");

  auto const il = get_interval_list(tr, allowed_events, infra);

  auto const start_time = tr.first_departure();
  for (auto const& event : il.front().events_) {
    utls::sassert(il.front().is_halt(),
                  "First event must be the departure halt.");
    event_reached(tr.sequence_points_.front().departure_, start_time,
                  event.node_->element_);
  }

  runtime_result current;
  for (auto inter_idx = 1U; inter_idx < il.size(); ++inter_idx) {
    auto const& prev_interval = il[inter_idx - 1];
    auto const& interval = il[inter_idx];

    auto const interval_length = interval.distance_ - prev_interval.distance_;
    utls::sassert(!is_zero(interval_length), "No intervals with length 0.");

    auto const p =
        get_runtime_phases(current, prev_interval, interval, tr.physics_);

    auto const interval_start_departure =
        start_time + si_to_relative_time(current.time_);

    current.speed_ = p.accel_.speed_;
    current.speed_ = p.has_coast_phase() ? p.coast_.speed_ : current.speed_;
    current.speed_ = p.has_deaccel_phase() ? p.deaccel_.speed_ : current.speed_;
    current.time_ += p.accel_.time_ + p.coast_.time_ + p.deaccel_.time_;
    current.distance_ +=
        p.accel_.distance_ + p.coast_.distance_ + p.deaccel_.distance_;

    auto const interval_end_arrival =
        start_time + si_to_relative_time(current.time_);
    auto interval_end_departure = interval_end_arrival;

    if (interval.is_halt()) {
      auto const earliest_possible_dep =
          interval_end_arrival + interval.min_stop_time();
      auto const planned_dep = interval.departure();

      interval_end_departure = std::max(planned_dep, earliest_possible_dep);

      auto const stand_time = interval_end_departure - interval_end_arrival;
      current.time_ += relative_to_si_time(stand_time);
    }

    determine_event_timestamps(prev_interval, interval,
                               interval_start_departure, interval_end_arrival,
                               interval_end_departure, p, event_reached);

    utl::verify(current.speed_ <= interval.limit_right_,
                "Going over speed limit is not allowed!");
  }
}

timestamps runtime_calculation(train const& tr, infrastructure const& infra,
                               infra::type_set const& record_types) {
  timestamps ts;

  auto const event_reached = [&ts](relative_time const arrival,
                                   relative_time const departure,
                                   element_ptr element) {
    utls::sassert(ts.times_.empty() || valid(arrival),
                  "Only the first timestamp can have invalid arrival");
    utls::sassert(departure >= arrival);
    // TODO(julian) reenable this, after improving the condition.
    // elements on the same kilometrage do not necessarily have to be
    // in ascending order
    // utls::sassert(ts.times_.empty() || arrival >= ts.times_.back().departure_);

    if (element->is(type::HALT)) {
      ts.halt_indices_.push_back(ts.times_.size());
    }

    ts.times_.emplace_back(arrival, departure, element);
  };

  runtime_calculation(tr, infra, event_reached, record_types);

  return ts;
}

}  // namespace soro::runtime
