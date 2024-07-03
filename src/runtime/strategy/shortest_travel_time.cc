#include "soro/runtime/strategy/shortest_travel_time.h"

#include <algorithm>
#include <iterator>
#include <vector>

#include "utl/verify.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/is_sorted.h"

#include "soro/base/time.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/type.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/timetable/train.h"

#include "soro/runtime/common/conversions.h"
#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/signal_time.h"
#include "soro/runtime/common/train_state.h"
#include "soro/runtime/driver/command.h"
#include "soro/runtime/physics/rk4/accelerate.h"
#include "soro/runtime/physics/rk4/brake.h"
#include "soro/runtime/physics/rk4/cruise.h"

namespace soro::runtime {

using namespace soro::rs;
using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime::rk4;

si::time get_arrival(record const& record, train_state const& state,
                     command const& command, interval const& i,
                     train_physics const& tp) {
  if (record.dist_.is_zero()) return si::time::zero();
  if (record.dist_ == state.dist_ + command.dist_) return command.time_;

  auto result = si::time::invalid();

  auto const deaccel =
      tp.braking_deaccel(i.infra_limit(), i.bwp_limit(), i.brake_path_length());
  utls::sassert(deaccel < si::accel::zero(),
                "braking deacceleration must be negative");

  auto const dist = record.dist_ - state.dist_;
  utls::sassert(si::length::zero() <= dist && dist <= command.dist_,
                "offset dist must be in range of the command");

  switch (command.action_) {
    case command::action::accelerate: {
      auto const accel =
          accelerate(state.speed_, i.speed_limit(tp), i.target_speed(tp),
                     i.length(), deaccel, i.slope(), dist, tp);
      result = accel.time_;
      break;
    }

    case command::action::brake: {
      // compute result = -v/a - sqrt((v^2/a^2) - (2 * (0 - x) / a))

      // -v/a
      result = -(state.speed_ / deaccel);

      // v(^2 / a^2) - (-2x / a)
      auto const root_inner =
          (state.speed_.pow<2>() / deaccel.template pow<2>()) -
          ((2 * -dist) / deaccel);

      if (root_inner > decltype(root_inner)::zero()) {
        result -= root_inner.sqrt();
      } else {
        utls::sassert(root_inner.is_zero(), "root_inner must be zero");
      }

      break;
    }

    case command::action::coast: {
      throw utl::fail("not implemented");
    }

    case command::action::cruise: {
      result = dist / command.speed_;
      break;
    }

    case command::action::halt: {
      result = si::time::zero();
      break;
    }

    case command::action::invalid: {
      throw utl::fail("got invalid command");
    }
  }

  utls::ensure(si::time::zero() <= result && result <= command.time_,
               "result must be in time range");

  return result;
}

std::vector<drive_event> get_events(train_state const initial,
                                    commands const& commands,
                                    interval const& interval,
                                    train_physics const& tp) {
  utls::expect(
      utls::is_sorted(interval.records(),
                      [](auto&& r1, auto&& r2) { return r1.dist_ < r2.dist_; }),
      "records not sorted by dist");

  std::vector<drive_event> result;

  if (commands.empty()) {
    for (auto const& record : interval.records()) {
      result.emplace_back(record.node_, si::time::zero());
    }
    return result;
  }

  auto current = initial;

  // zero out the time value, so that every event timestamp is given
  // as an offset from the start of the interval
  current.time_ = si::time::zero();

  auto current_command = std::begin(commands);

  for (auto const& record : interval.records()) {
    while (std::next(current_command) != std::end(commands) &&
           current.dist_ + current_command->dist_ <= record.dist_) {
      current.dist_ += current_command->dist_;
      current.time_ += current_command->time_;
      current.speed_ = current_command->speed_;
      current_command += 1;
    }

    auto const event_arrival =
        current.time_ +
        get_arrival(record, current, *current_command, interval, tp);

    utls::sassert(si::time::zero() <= event_arrival &&
                      event_arrival <= current.time_ + current_command->time_,
                  "event arrival must be in time range");

    result.emplace_back(record.node_, event_arrival);
  }

  return result;
}

void shortest_travel_time::update(interval const& interval, train const& train,
                                  train::trip const& trip,
                                  train_state const& initial,
                                  signal_time const& signal_time) {
  if (signal_time.time_ == ZERO<absolute_time>) return;

  if (!approaching_ && interval.starts_on_signal(type::APPROACH_SIGNAL) &&
      interval.start_signal().id_ == signal_time.approach_) {
    auto const current_time =
        trip.anchor_ + train.start_time_ + to_relative(initial.time_);
    approaching_ = signal_time.time_ >= current_time;
  }

  if (approaching_ && interval.starts_on_signal(type::MAIN_SIGNAL)) {
    approaching_ = false;
  }

  if (approaching_ &&
      interval.next_signal().in_sight(interval.start_distance())) {
    auto const current_time =
        trip.anchor_ + train.start_time_ + to_relative(initial.time_);
    approaching_ = signal_time.time_ >= current_time;
  }
}

si::speed shortest_travel_time::get_max_speed(interval const& interval,
                                              train_physics const& tp) const {
  utls::expect(interval.speed_limit(tp).is_valid());
  utls::expect(interval.signal_limit().is_valid());

  if (!approaching_) return interval.speed_limit(tp);

  return std::min(interval.speed_limit(tp), interval.signal_limit());
}

si::speed shortest_travel_time::get_target_speed(
    interval const& interval, train_physics const& tp) const {
  utls::expect(interval.target_speed(tp).is_valid());
  utls::expect(interval.target_signal_speed().is_valid());

  if (!approaching_) return interval.target_speed(tp);

  return std::min(interval.target_speed(tp), interval.target_signal_speed());
}

si::accel get_braking_deaccel(interval const& interval,
                              train_physics const& tp) {
  return tp.braking_deaccel(interval.infra_limit(), interval.bwp_limit(),
                            interval.brake_path_length());
}

delta shortest_travel_time::drive(train_state const& initial, train_safety*,
                                  interval const& interval, train const& train,
                                  train::trip const& trip,
                                  signal_time const& signal_time) {
  this->update(interval, train, trip, initial, signal_time);

  if (interval.length().is_zero()) {
    auto result = delta::zero();
    result.events_ = get_events(initial, {}, interval, train.physics_);
    return result;
  }

  auto const& tp = train.physics_;

  commands commands;

  // all train states are initialized to zero
  train_state accel;
  train_state cruise;
  train_state brake;

  auto const deaccel = get_braking_deaccel(interval, tp);
  auto const max_speed = get_max_speed(interval, tp);
  auto const target_speed = get_target_speed(interval, tp);

  utls::sassert(initial.speed_ <= max_speed, "no speeding");

  auto const init_tractive = tp.tractive_force(initial.speed_);
  auto const init_resistive =
      tp.resistive_force(initial.speed_, interval.slope());

  auto const not_at_max_speed = initial.speed_ < max_speed;
  auto const cannot_cruise = init_tractive < init_resistive;
  auto const should_accel = not_at_max_speed || cannot_cruise;
  if (should_accel) {
    accel =
        accelerate(initial.speed_, max_speed, target_speed, interval.length(),
                   deaccel, interval.slope(), interval.length(), tp);
  }

  auto const brake_from = should_accel ? accel.speed_ : max_speed;
  auto const should_brake = brake_from > target_speed;
  if (should_brake) {
    brake = rk4::brake(brake_from, target_speed, deaccel);
  }

  auto const current_speed = should_accel ? accel.speed_ : initial.speed_;

  auto const tractive = tp.tractive_force(current_speed);
  auto const resistive = tp.resistive_force(current_speed, interval.slope());

  auto const cruise_force = tractive > resistive;
  auto const cruise_length = interval.length() - accel.dist_ - brake.dist_;
  auto const can_cruise = cruise_force && cruise_length > si::length::zero();
  auto const should_cruise = can_cruise && current_speed == max_speed;
  if (should_cruise) {
    cruise = rk4::cruise(max_speed, cruise_length);
  }

  if (should_accel) {
    commands.emplace_back(command::action::accelerate, accel);
  }

  if (should_cruise) {
    commands.emplace_back(command::action::cruise, cruise);
  }

  if (should_brake) {
    commands.emplace_back(command::action::brake, brake);
  }

  utls::ensure(!commands.empty(), "no commands generated");
  utls::ensure(commands.size() < 4, "too many commands generated");

  auto const new_speed = should_brake
                             ? brake.speed_
                             : (should_cruise ? cruise.speed_ : accel.speed_);
  delta result;
  result.time_ = accel.time_ + cruise.time_ + brake.time_;
  result.dist_ = accel.dist_ + cruise.dist_ + brake.dist_;
  result.speed_ = new_speed - initial.speed_;
  result.events_ = get_events(initial, commands, interval, tp);

  utls::ensure(result.dist_ == interval.length(),
               "must exactly cover interval");
  utls::ensure(new_speed <= interval.target_speed(tp), "no speeding");
  utls::ensure(result.events_.size() == interval.records().size(),
               "must have an event for each record");

  return result;
}

}  // namespace soro::runtime
