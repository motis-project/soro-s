#pragma once

#include "driving_regime.h"

namespace soro::runtime {

enum general_driving_types {
  COASTING,
  CRUISING,
  MAX_BRAKING,
  MAX_ACCELERATION
};

class general_driving : public driving_regime {

private:
  /**
   * A utility function, only used in and by general_driving, when simulate_on
   * is called.
   *
   * @see general_driving::simulate_on for params description
   */
  static bool check_simulate_on_params(std::vector<interval> const& intervals,
                                       si::length start, si::length end,
                                       si::speed vel_start, si::length offset,
                                       si::time time_offset,
                                       si::length step_size,
                                       si::speed allowed_max_velocity);

public:
  enum general_driving_types type_ = CRUISING;

  // runtime results vector describing the last simulation result
  std::vector<runtime_result> last_run_;

  general_driving(si::length const start, si::length const end, si::speed vel0,
                  si::time t0, general_driving_types type)
      : driving_regime(start, end, vel0, t0), type_(type) {}

  /**
   * Determines the minimal and maximal speed of this general driving regime.
   *
   * @return  a tuple of two speed values describing the speed range of this
   * general driving regime. (min_speed, max_speed).
   */
  std::tuple<si::speed, si::speed> get_speed_range();

  /**
   * Determines the current acceleration of the train, taking into account the
   * selected driving regime and the current velocity of the train.
   *
   * This type of acceleration determination allows the use of a single class
   * for all driving regimes of a train, which are basically always related to
   * acceleration or tractive forces.
   *
   * TYPES:
   *  - MAX_ACCELERATION: Acceleration defined regarding the current velocity.
   *  - MAX_BRAKING: currently implemented as soft braking with a current
   *    acceleration.
   *  - COASTING: acceleration is set to zero.
   *  - CRUISING: currently implemented as a standalone class. To update this
   *    implementation to a acceleration based one set: a = sum of resistances
   *
   * @param tv the used train physics
   * @param current_velocity the current velocity (m/s)
   * @return the acceleration of a train at a given velocity (m/s^2)
   */
  // NOLINTNEXTLINE
  si::acceleration get_acceleration(rs::train_physics const& tv,
                                    si::speed current_velocity) const;

  /**
   * Calculation of the time at which an object with acceleration a,
   * initial velocity v0 traveled the distance dx.
   *
   * Searches possible solutions for 0 = 0.5 * a * t^2 + v0 * t + dx
   *
   * @param acc acceleration in m/s^2 during dt
   * @param vel velocity in m/s at t0
   * @param distance traveled distance with given acceleration
   * @return a tuple of two time possible periods given in seconds
   */
  static std::tuple<si::time, si::time> get_acceleration_duration(
      si::acceleration acc, si::speed vel, si::length distance);

  /**
   * A general simulation implementation that works for all general driving
   * regimes such as MAX_ACCELERATION, MAX_BRAKING, COASTING and CRUISING.
   * It returns a vector of runtime_results, which gives details of the
   * simulation of the simulated general driving regime from start to end with
   * given initial definitions. Therefore this methods simulates the trains
   * acceleration from straight forward from the start to the end.
   *
   * A simulation is not(!) fixed to the regimes start and end point.
   *
   * The simulate_on method has the following requirements:
   * - the intervals must all have the same speed_limit; therefore this function
   * works on constant speed intervals, which helps avoiding complex speed limit
   * checks
   * - start must be before end
   * - start and end must be divisible by step_size without a remainder
   * - vel_start, offset, time_offset and step_size must be greater or equal to
   * zero
   *
   * @todo it may be possible to pass most of the arguments as key-value
   *   structure
   *
   * @param tp the used train physics during this simulation.
   * @param intervals intervals the train drives on during this simulation.
   * @param start the starting point of this simulation in meters.
   * @param end the end point of this simulation in meters.
   * @param vel_start the trains velocity at start.
   * @param offset a offset in meters to get the correct intervals (since
   * intervals only have a distance, but not a concrete start and end).
   * @param time_offset a offset to calculate the correct time (if zero: time
   * starts at zero, otherwise at time_offset).
   * @param step_size a step_size in meters, defines the size of runtime_result
   * struct.
   * @return a vector/list of runtime_results representing the result of this
   * simulation.
   */
  std::vector<runtime_result> simulate_on(
      rs::train_physics const& tp, std::vector<interval> const& intervals,
      si::length start, si::length end, si::speed vel_start, si::length offset,
      si::time time_offset, si::length step_size) override;

  /**
   * A general simulation implementation that works only for driving regimes
   * with a constant negative acceleration, such as MAX_BRAKING and COASTING.
   * In comparison to general_driving::simulate_on this function simulates the
   * trains acceleration from end with given end information (s.a. the velocity
   * at end) to start. Therefore it is reversed.
   *
   * Known: acc(i-1), v(i), pos(i-1), pos(i)
   * Calculated: v(i-1), time from i-1 to i
   * Where i is the current position and i-1 the directly preceding position
   *
   * @remark the acceleration must be independent of the velocity, since the
   * velocity is unknown (therefore to be calculated here) an acceleration that
   * depends on the velocity would yield more complex calculations.
   * @remark the acceleration must be negative. However, the accelerate_reverse
   * function can easily updated to work with non-negative accelerations
   *
   * @see general_driving::simulate_on
   */
  std::vector<runtime_result> simulate_reverse_on(
      rs::train_physics const& tp, std::vector<interval> const& intervals,
      si::length start, si::length end, si::speed vel_end, si::length offset,
      si::time time_offset, si::length step_size) override;

  /**
   * Executes general_driving::simulate_on and outputs the runtime_result vector
   * obtained there. In general_driving::run the parameters defined in the
   * general_driving_regime are used to define start and end.
   *
   * @see general_driving::simulate_on
   */
  std::vector<runtime_result> run(rs::train_physics const& tp,
                                  std::vector<interval> const& intervals,
                                  si::speed vel_start, si::length offset,
                                  si::time time_offset, si::length step_size);
};

// defines a vector of general driving regimes as general_driving_regimes
using general_driving_regimes = std::vector<general_driving>;

}  // namespace soro::runtime