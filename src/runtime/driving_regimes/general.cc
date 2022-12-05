#include "soro/runtime/driving_regimes/general.h"

#include "soro/runtime/driving_regimes/physics.h"

namespace soro::runtime {

bool general_driving::check_simulate_on_params(
    interval_list const& intervals, si::length const start,
    si::length const end, si::speed const vel_start, si::length const offset,
    si::time const time_offset, si::length const step_size,
    si::speed const allowed_max_velocity) {

  utl::verify(!intervals.empty(),
              "general_driving::check_simulate_on_params: There must be at "
              "least one interval in intervals.");
  for (auto const& interval : intervals) {
    utl::verify(interval.speed_limit_ == intervals[0].speed_limit_,
                "general_driving::check_simulate_on_params: All intervals "
                "speed limits must be the same.");
  }

  utl::verify(start >= si::length{0.0F},
              "general_driving::check_simulate_on_params: start must be "
              "greater than or equal to zero.");
  utl::verify(fmod(start.val_, step_size.val_) == 0,
              "general_driving::check_simulate_on_params: start must be a "
              "multiple of step_size.");

  utl::verify(end >= si::length{0.0F},
              "general_driving::check_simulate_on_params: end must be "
              "greater than or equal to zero.");
  utl::verify(fmod(end.val_, step_size.val_) == 0,
              "general_driving::check_simulate_on_params: end must be a "
              "multiple of step_size.");
  utl::verify(vel_start <= allowed_max_velocity,
              "general_driving::check_simulate_on_params: Given start velocity "
              "is greater than the allowed maximal velocity.");

  utl::verify(
      vel_start >= si::speed{0.0F},
      "general_driving::check_simulate_on_params: starting velocity must be "
      "greater than or equal to zero.");
  utl::verify(offset >= si::length{0.0F},
              "general_driving::check_simulate_on_params: offset must be "
              "greater than or equal to zero.");
  utl::verify(time_offset >= si::time{0.0F},
              "general_driving::check_simulate_on_params: time_offset must be "
              "greater than or equal to zero.");
  utl::verify(step_size >= si::length{0.0F},
              "general_driving::check_simulate_on_params: step_size must be "
              "greater than or equal to zero.");

  return true;
}

/**
 * Determines the speed range of this driving regime by using its last_run_
 * runtime_results vector containing speed, time and position.
 *
 * @return a tuple of two speed values: (min speed, max speed)
 */
std::tuple<si::speed, si::speed> general_driving::get_speed_range() {
  auto min_speed = si::speed{INT_MAX};
  auto max_speed = si::speed{INT_MIN};

  for (auto& rr : this->last_run_) {
    if (rr.speed_ < min_speed) {
      min_speed = rr.speed_;
    }

    if (rr.speed_ > max_speed) {
      max_speed = rr.speed_;
    }
  }

  return std::make_tuple(min_speed, max_speed);
}

/**
 * Determines the current acceleration of the given train regarding its active
 * driving regime.
 *
 * @param tp the used train physics
 * @param current_velocity the current velocity (m/s)
 * @return the acceleration of a train at a given velocity (m/s^2)
 */
si::acceleration general_driving::get_acceleration(
    rs::train_physics const& tp, si::speed current_velocity) const {

  // determine the acceleration caused by resistance force
  si::force const resistive_force = tp.resistive_force(current_velocity);
  si::acceleration const resistance_acc = resistive_force / tp.weight();

  si::acceleration train_acc;

  // determine the acceleration of the train at current velocity
  switch (this->type_) {
    case CRUISING: train_acc = resistance_acc; break;
    case COASTING: train_acc = si::acceleration{0.0F}; break;
    case MAX_BRAKING: train_acc = tp.deacceleration(); break;
    case MAX_ACCELERATION:
      train_acc = tp.tractive_force(current_velocity) / tp.weight();
      break;
  }

  return train_acc - resistance_acc;
}

/**
 * Calculates the time in seconds at which an object with an given acceleration
 * acc and a starting velocity of v0 has traveled the distance distance
 *
 * Searches possible solutions for 0 = 0.5 * acc * t^2 + v0 * t + distance by
 * solving the pq-equation
 * - p = (2 * v0) / acc; [p] = s
 * - q = (2 (x0 - x)) / acc; [q] = s^2 * with distance = (x0 - x)
 * Non-valid zero points are output as nan (can be checked by std::is_nan(x)
 *
 * This function has some requirements:
 * - acceleration can be any real number
 * - velocity can be any real number (but should be greater or equal to zero)
 * - distance to travel must be given as an absolute value:
 *   x0 < x => distance < 0 but give it as positive number!
 *
 * @see
 * https://en.wikipedia.org/wiki/Quadratic_equation#Reduced_quadratic_equation
 *
 * @param acc acceleration in m/s^2 during dt
 * @param vel velocity in m/s at t0
 * @param distance traveled distance with given acceleration
 * @return a tuple of two time possible periods given in seconds
 */
std::tuple<si::time, si::time> general_driving::get_acceleration_duration(
    si::acceleration const acc, si::speed const vel,
    si::length const distance) {
  si::time const pq_p = (2.0F * vel) / acc;
  si::time const pq_p_2 = pq_p * 0.5F;
  si::time const pq_p_2_neg = pq_p_2 * -1.0F;

  // squared time(s)
  auto const pq_p_2_squared = pq_p_2 * pq_p_2;
  auto const pq_q = (-2.0F * distance) / acc;

  // sqrt of a squared time should be si::time (currently no strong_typing
  // support)
  return std::make_tuple(
      pq_p_2_neg + si::time{sqrt((pq_p_2_squared - pq_q).val_)},
      pq_p_2_neg - si::time{sqrt((pq_p_2_squared - pq_q).val_)});
}

std::vector<runtime_result> general_driving::simulate_on(
    rs::train_physics const& tp, interval_list const& intervals,
    si::length const start, si::length const end, si::speed const vel_start,
    si::length const offset, si::time const time_offset,
    si::length const step_size) {

  // a train is allowed to drive as fast as the speed_limit, but not faster than
  // his max_speed
  auto allowed_max_velocity =
      std::min(intervals[0].speed_limit_, tp.max_speed());

  this->check_simulate_on_params(intervals, start, end, vel_start, offset,
                                 time_offset, step_size, allowed_max_velocity);

  si::length current_position = start;
  si::time current_time = time_offset;
  si::speed current_velocity = vel_start;
  std::vector<runtime_result> results = {};

  results.emplace_back(current_time, current_position, current_velocity);

  while (current_position < end) {
    auto const current_acceleration =
        this->get_acceleration(tp, current_velocity);
    auto const acc_result =
        accelerate(current_acceleration, current_velocity, current_position,
                   current_position + step_size);

    // check and update next velocity
    auto next_velocity = std::get<1>(acc_result);
    if (next_velocity > allowed_max_velocity) {
      // a train is not allowed to have a velocity higher than the allowed
      // maximal velocity
      next_velocity = allowed_max_velocity;
    }

    if (next_velocity < si::speed{0.0F}) {
      // train is not allowed to have a velocity of less than zero
      next_velocity = si::speed{0.0F};
    }

    current_position += step_size;
    current_time += std::get<0>(acc_result);
    current_velocity = next_velocity;

    results.emplace_back(current_time, current_position, current_velocity);
  }

  return results;
}
std::vector<runtime_result> general_driving::simulate_reverse_on(
    rs::train_physics const& tp, interval_list const& intervals,
    si::length const start, si::length const end, si::speed const vel_end,
    si::length const offset, si::time const time_offset,
    si::length const step_size) {
  // a train is allowed to drive as fast as the speed_limit, but not faster than
  // his max_speed

  auto allowed_max_velocity =
      std::min(intervals[0].speed_limit_, tp.max_speed());

  this->check_simulate_on_params(intervals, start, end, vel_end, offset,
                                 time_offset, step_size, allowed_max_velocity);

  si::length current_position = end;
  si::time current_time = {0.0F};
  si::speed current_velocity = vel_end;
  std::vector<runtime_result> results = {};

  results.emplace_back(current_time, current_position, current_velocity);

  while (current_position > start) {
    /**
     * Why is this possible when acceleration actually depends on the previous
     * velocity? Basically, acceleration, previous velocity and required time
     * for a step are unknown.
     * However, in reverse simulation the driving regime MAX_ACCELERATION and
     * CRUISING is not taken into account. Therefore, the acceleration depends
     * only on the train resistance and the current track property and is thus
     * independent of the previous velocity over all intervals. Track properties
     * can be taken into account based on knowledge of the exact location.
     * However, track properties are currently unconsidered.
     */
    si::acceleration const current_acceleration =
        this->get_acceleration(tp, current_velocity);
    auto const acc_result =
        accelerate_reverse(current_acceleration, current_velocity,
                           current_position - step_size, current_position);

    // check and update next velocity
    auto prev_velocity = std::get<1>(acc_result);
    if (prev_velocity > allowed_max_velocity) {
      // a train is not allowed to have a velocity higher than the allowed one
      prev_velocity = allowed_max_velocity;
    }

    if (prev_velocity < si::speed{0.0F}) {
      // a train is not allowed to have a negative velocity
      prev_velocity = si::speed{0.0F};
    }

    current_position -= step_size;
    current_time = std::get<0>(acc_result);
    current_velocity = prev_velocity;

    results.emplace_back(current_time, current_position, current_velocity);
  }

  // reverse results to get runtime_results from start to end
  std::reverse(results.begin(), results.end());
  si::time accumulated_required_time = time_offset;

  // update/set time of every runtime_results
  // before: time needed for step: delta_time
  // after: accumulated time after step
  for (auto& rr : results) {
    auto const rr_time = rr.time_;
    rr.time_ = accumulated_required_time;
    accumulated_required_time += rr_time;
  }

  return results;
}

std::vector<runtime_result> general_driving::run(rs::train_physics const& tp,
                                                 interval_list const& intervals,
                                                 si::speed vel_start,
                                                 si::length offset,
                                                 si::time time_offset,
                                                 si::length step_size) {
  runtime_results rr;

  // use simulate_reverse_on if acceleration is negative
  // MAX_BRAKING (DECELERATE) has negative acceleration
  // COASTING may have negative acceleration, however frictionless train's
  // COASTING equals CRUISING
  if (this->type_ == MAX_BRAKING ||
      (this->type_ == COASTING &&
       this->get_acceleration(tp, vel_start) < si::acceleration{0.0F})) {
    rr = this->simulate_reverse_on(tp, intervals, this->start_, this->end_,
                                   vel_start, offset, time_offset, step_size);
  } else {
    rr = this->simulate_on(tp, intervals, this->start_, this->end_, vel_start,
                           offset, time_offset, step_size);
  }
  this->last_run_ = rr;
  return rr;
}

}  // namespace soro::runtime
