#include "soro/runtime/runtime.h"

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

// size_t get_next_halt_idx(train const& train_run, size_t const current_idx) {
//   for (auto idx = current_idx + 1; idx < train_run.entries_.size(); ++idx) {
//     if (train_run.entries_[idx].halt()) {
//       return idx;
//     }
//   }
//
//   return train_run.entries_.size() - 1;
// }

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
                          struct interval const& next_interval,
                          rs::train_physics const& tp, bool const halt) {
  auto const interval_length = interval.distance_ - prev_interval.distance_;

  auto const current_max_speed =
      std::min(tp.max_speed(), interval.speed_limit_);
  auto const next_max_speed =
      std::min(tp.max_speed(), next_interval.speed_limit_);

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
  auto const target_speed = halt ? ZERO<speed> : next_max_speed;
  if (current.speed_ > target_speed) {
    p.deaccel_results_ = brake(tp, current.speed_, target_speed);
    utl::verify(!p.deaccel_results_.empty(), "Could not get any brake results");
    p.deaccel_ = p.deaccel_results_.back();
    utl::verify(p.deaccel_.time_ > ZERO<time>, "Wrong");

    utl::verify(p.deaccel_.distance_ <= interval_length,
                "Not enough distance to deaccel");
  }

  // if acceleration distance and deacceleration distance is larger than
  // available distance dont accelerate to top speed, but stop before.
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

template <typename EventReachedFn>
void determine_event_timestamps(struct interval const& prev_interval,
                                struct interval const& interval,
                                unixtime const& interval_start_ts,
                                unixtime const& end_arrival,
                                unixtime const& end_departure,
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
    auto const time_stamp = interval_start_ts + duration(as_s(event_time));

    event_reached(time_stamp, time_stamp, event.node_->element_);
  }
}

template <typename EventReachedFn>
void runtime_calculation(train const& tr, infrastructure const& infra,
                         EventReachedFn const& event_reached,
                         type_set const& allowed_events) {
  auto il = get_interval_list(tr, allowed_events, BORDER_TYPES, infra);

  for (auto const& event : il.front().events_) {
    auto const dep = tr.stop_times_.front().departure_;
    event_reached(INVALID_TIME, dep, event.node_->element_);
  }

  auto const start_ts = tr.first_departure();
  //  auto next_halt_idx = get_next_halt_idx(train_run, 0);

  runtime_result current;
  for (auto inter_idx = 1UL; inter_idx < il.size() - 1; ++inter_idx) {
    auto const& prev_interval = il[inter_idx - 1];
    auto const& interval = il[inter_idx];
    auto const& next_interval = il[inter_idx + 1];

    auto const interval_length = interval.distance_ - prev_interval.distance_;
    if (interval_length == ZERO<length>) {
      continue;
    }

    auto const p =
        get_runtime_phases(current, prev_interval, interval, next_interval,
                           tr.physics_, interval.halt_);

    unixtime const interval_start_ts = start_ts + duration(as_s(current.time_));

    current.speed_ = p.accel_.speed_;
    current.speed_ = p.has_coast_phase() ? p.coast_.speed_ : current.speed_;
    current.speed_ = p.has_deaccel_phase() ? p.deaccel_.speed_ : current.speed_;
    current.time_ += p.accel_.time_ + p.coast_.time_ + p.deaccel_.time_;
    current.distance_ +=
        p.accel_.distance_ + p.coast_.distance_ + p.deaccel_.distance_;

    unixtime const interval_end_arrival =
        start_ts + duration(as_s(current.time_));
    unixtime interval_end_departure = interval_end_arrival;

    // determine stand time for the halt, except the last one
    if (interval.halt_ && inter_idx != il.size() - 2) {
      //      auto const& halt_entry = train_run.entries_[next_halt_idx];

      //      auto const earliest_possible_dep =
      //          interval_end_arrival + halt_entry.min_stop_time_;
      //      auto const planned_dep = halt_entry.departure_;
      //      interval_end_departure = std::max(planned_dep,
      //      earliest_possible_dep);
      //
      //      time stand_time{
      //          static_cast<float>(interval_end_departure -
      //          interval_end_arrival)};
      //      current.time_ += stand_time;

      //      next_halt_idx = get_next_halt_idx(train_run, next_halt_idx);
    }

    if (inter_idx == il.size() - 2) {
      interval_end_departure = INVALID_TIME;
    }

    determine_event_timestamps(prev_interval, interval, interval_start_ts,
                               interval_end_arrival, interval_end_departure, p,
                               event_reached);

    utl::verify(current.speed_ <= next_interval.speed_limit_,
                "Going over speed limit is not allowed!");
  }
}

timestamps runtime_calculation(train const& tr, infrastructure const& infra,
                               infra::type_set const& record_types) {
  timestamps ts;

  auto const event_reached = [&ts](utls::unixtime const& arrival,
                                   utls::unixtime const& departure,
                                   element_ptr element) {
    utl::verify(arrival > 0 && departure > 0, "Negative timestamps!");
    if (element->is(type::HALT)) {
      ts.halt_indices_.push_back(ts.times_.size());
    }

    ts.times_.emplace_back(arrival, departure, element);
  };

  runtime_calculation(tr, infra, event_reached, record_types);

  return ts;
}

template <typename EventReachedFn>
runtime_result calculate_interval(
    interval const& prev_interval, interval const& inter,
    interval const& next_interval, train const& tr, runtime_result current,
    unixtime const& start_ts, EventReachedFn&& event_reached,
    utls::unixtime const& go_time, utls::duration const& min_stand_time,
    utls::unixtime const& earliest_departure,
    utls::duration const& extra_stand_time) {
  //  auto const& tv = train_run.get_variant();

  auto const interval_length = inter.distance_ - prev_interval.distance_;
  if (interval_length == ZERO<length>) {
    return current;
  }

  unixtime const interval_start_ts = start_ts + duration(as_s(current.time_));

  auto const halt_at_main =
      interval_start_ts < go_time &&
      utls::contains(prev_interval.elements_, type::APPROACH_SIGNAL);
  auto const halt = inter.halt_ || halt_at_main;

  auto const& p = get_runtime_phases(current, prev_interval, inter,
                                     next_interval, tr.physics_, halt);

  current.speed_ = p.accel_.speed_;
  current.speed_ = p.has_coast_phase() ? p.coast_.speed_ : current.speed_;
  current.speed_ = p.has_deaccel_phase() ? p.deaccel_.speed_ : current.speed_;
  current.time_ += p.accel_.time_ + p.coast_.time_ + p.deaccel_.time_;
  current.distance_ +=
      p.accel_.distance_ + p.coast_.distance_ + p.deaccel_.distance_;

  unixtime const interval_end_arrival =
      start_ts + duration(as_s(current.time_));
  unixtime const interval_end_departure = interval_end_arrival;

  //  assert((inter.halt_ &&
  //         !utls::contains(inter.elements_, type::APPROACH_SIGNAL)) ||
  //         (inter.halt_ && utlsk));

  assert(static_cast<size_t>(inter.halt_) +
             static_cast<size_t>(
                 utls::contains(inter.elements_, type::APPROACH_SIGNAL)) <
         2);

  if (inter.halt_) {
    auto const earliest_possible_dep = interval_end_arrival + min_stand_time;

    auto const departure =
        std::max({earliest_possible_dep,
                  earliest_departure == INVALID_TIME
                      ? utls::EPOCH
                      : earliest_departure + extra_stand_time,
                  go_time == INVALID_TIME ? utls::EPOCH : go_time});

    time const stand_time{static_cast<float>(departure - interval_end_arrival)};
    current.time_ += stand_time;
  } else if (halt_at_main) {
    current.time_ += time{static_cast<float>(go_time - interval_end_arrival)};
  }

  determine_event_timestamps(prev_interval, inter, interval_start_ts,
                             interval_end_arrival, interval_end_departure, p,
                             event_reached);

  utl::verify(current.speed_ <= next_interval.speed_limit_,
              "Going over speed limit is not allowed!");

  return current;
}

discrete_scenario runtime_calculation_ssr(
    train const& tr, infrastructure const& infra, ir_id const ir_id,
    utls::unixtime const start_time, si::speed const init_speed,
    utls::unixtime const go_time, utls::duration const extra_stand_time) {
  discrete_scenario result;

  auto il = get_interval_list(tr, {type::HALT, type::MAIN_SIGNAL, type::EOTD},
                              BORDER_TYPES, infra);

  auto const& ssr = infra->interlocking_.interlocking_routes_[ir_id];

  //  auto const is_last_ssr = ssr_id == tr.ssr_run_.path_.back()->id_;
  auto const is_first_ssr = ir_id == tr.path_.front()->id_;

  auto const& signal_eotd = ssr->nodes(ssr->signal_eotd_);

  auto const ir_offset = utls::find_if_position(
      tr.path_, [&](auto&& ssr_ptr) { return ssr_ptr->id_ == ir_id; });

  auto const min_stand_time = tr.stop_times_[ir_offset].min_stop_time_;
  auto const earliest_departure = tr.stop_times_[ir_offset].departure_;

  auto const from_interval =
      is_first_ssr ? 0 : utls::find_if_position(il, [&](auto const& interval) {
        return utls::any_of(interval.events_, [&](auto const& event) {
          return event.node_->id_ == ssr->nodes().front()->id_;
        });
      });

  auto const to_interval =
      from_interval +
      utls::find_if_position(
          std::cbegin(il) + static_cast<std::ptrdiff_t>(from_interval),
          std::cend(il),
          [&](auto const& interval) {
            return utls::any_of(interval.events_, [&](auto const& event) {
              return event.node_->id_ == ssr->nodes().back()->id_;
            });
          }) +
      1;

  std::cout << "From: " << from_interval << '\n';
  std::cout << "To: " << to_interval << '\n';

  utls::slice_clasp(il, from_interval, to_interval + 1);
  std::cout << "Elements[to]:\n";
  for (auto const& element : il.back().elements_) {
    std::cout << get_type_str(element) << '\n';
  }
  std::cout << "Elements[to - 1]:\n";
  for (auto const& element : il[il.size() - 2].elements_) {
    std::cout << get_type_str(element) << '\n';
  }

  runtime_result current;
  current.time_ = si::ZERO<si::time>;
  current.speed_ = init_speed;
  current.distance_ = si::ZERO<si::length>;

  auto const save_signal_eotd_time = [&](utls::unixtime const& arrival,
                                         utls::unixtime const&,
                                         element_ptr element) {
    if (element->id() == signal_eotd->element_->id()) {
      result.eotd_arrival_ = arrival;
    }
  };

  for (auto const [prev, inter, next] : utl::nwise<3>(il)) {
    //        unixtime const interval_start_ts =
    //            start_time + duration(as_s(current.time_));

    current = calculate_interval(prev, inter, next, tr, current, start_time,
                                 save_signal_eotd_time, go_time, min_stand_time,
                                 earliest_departure, extra_stand_time);
  }

  std::cout << start_time << '\n';
  std::cout << init_speed << '\n';
  std::cout << go_time << '\n';
  std::cout << "hi\n";

  result.end_arrival_ = start_time + duration(as_s(current.time_));
  result.end_speed_ = current.speed_;

  return result;
}

scenario_result runtime_calculation(tt::train const&, infra::ir_id const,
                                    utls::unixtime const, si::speed const,
                                    utls::duration const, utls::unixtime const,
                                    infra::infrastructure const&) {
  return scenario_result{};
}

}  // namespace soro::runtime
