#pragma once

#include "soro/runtime/driving_regimes/general.h"
#include "soro/runtime/interval.h"
#include "soro/runtime/runtime_physics.h"

namespace soro::runtime {

/**
 * A speed interval is a combination of directly connected intervals having the
 * same speed limit.
 *
 * It is defined by a start and an end in meter, a speed_limit in meters per
 * second that holds from start to end for all used intervals between
 * start and end.
 *
 * If the speed_interval has an halt it must be at the end of the
 * speed_interval, even if the next speed_interval would then has the same
 * speed_limit.
 *
 * The length is trivial and set to end - start. Since the start is always
 * before the end this value cannot be negative.
 */
struct speed_interval {
  speed_interval() = delete;
  speed_interval(si::length const start, si::length const end,
                 si::speed const speed_limit, interval_list const& intervals)
      : start_(start),
        end_(end),
        speed_limit_(speed_limit),
        intervals_(intervals) {}
  speed_interval(si::length const start, si::length const end,
                 si::speed const speed_limit, bool const halt,
                 interval_list const& intervals)
      : start_(start),
        end_(end),
        speed_limit_(speed_limit),
        halt_(halt),
        intervals_(intervals) {}

  si::length start_{si::ZERO<si::length>};
  si::length end_{si::ZERO<si::length>};
  si::speed speed_limit_{si::ZERO<si::speed>};

  int run_ctr_{0};

  bool halt_{false};

  interval_list intervals_;
  general_driving_regimes driving_regimes_{};

  si::length length() const;

  /**
   * Checks whether the speed interval contains a driving_regime of type
   * type or not.
   *
   * @param type a registered driving regime type.
   *
   * @return True, if driving regime type exists in the speed interval,
   * otherwise false.
   */
  bool has_driving_regime_type(general_driving_types type) const;

  /**
   * Determines the minimal and maximal valid speed of this speed interval
   * usable as cruising speed.
   *
   * @remark used the get_cruising_speed_range function of a driving_regime
   *
   * @param safe_range reduces speed_range to a safe range, such that the
   * cruising speed cannot change the final interval speed. Only works, if last
   * driving regime is either coasting or braking. Therefore, safe_range set to
   * true has no effect when using acceleration (and/or) cruising since it would
   * reduce the speed_range to a single value.
   *
   * @return a tuple of two speed values describing a speed range of possible
   * cruising speeds of this speed interval.
   */
  std::tuple<si::speed, si::speed> get_cruising_speed_range(
      bool safe_range = true);

  /**
   * Determines an interval in which the cruising speed is lower than the
   * current speed.
   *
   * @remark 1. The initial and final speeds must not be changed. Therefore,
   * they must be lower than the cruising speed (candidate).
   * @remark 2. A cruising interval can only exist if both a positive and a
   * negative acceleration occur in the speed interval under consideration.
   * @remark 3. Remarks 1 and 2 guarantee that a change in the train speed only
   * occurs in the considered speed interval and that there is no influence on
   * any preceding or following speed intervals in all calculations.
   *
   * @param cr_speed_cand the cruising speed for which the interval is to be
   * determined.
   * @return a interval in which the cruising speed is lower than the current
   * speed. Start and End must be in the speed interval. Otherwise {-1, -1}.
   */
  std::tuple<si::length, si::length> get_cruising_interval(
      si::speed cr_speed_cand) const;

  /**
   * Determines the current transit time.
   * The transit time is the time it takes the train to pass the speed interval.
   *
   * @return the transit time required during the last full run.
   */
  si::time get_transit_time() const;

  /**
   * @brief Returns the time given a specific position.
   *
   * @remark position must be a valid position in this speed interval.
   *
   * @param pos a specific position in this speed interval, si::length.
   * @return the (travel) time at the given position in this speed interval.
   */
  si::time get_time_at_position(si::length pos) const;

  /**
   * @brief Returns the travel time at the end of this speed interval.
   *
   * @remark Uses get_time_at_position(si::length) to retrieve the travel time
   * at end of this speed interval.
   *
   * @note Is integrated as a function in order to be able to react more easily
   * to changes in the travel time structure in the course of the
   * implementations, since changes only become necessary here.
   *
   * @return The (travel) time at the end of this speed interval.
   */
  si::time get_time_at_end() const;

  /**
   * @brief Returns the speed given a specific position.
   *
   * @remark position must be a valid position in this speed interval.
   *
   * @param pos a specific position in this speed interval, si::length.
   * @return the (travel) speed at the given position in this speed interval.
   */
  si::speed get_speed_at_position(si::length pos) const;

  /**
   * @brief Returns the type of the driving regime given a specific position.
   *
   * @remark position must be a valid position in this speed interval.
   *
   * @param pos a specific position in this speed interval, si::length
   * @return the driving regime tyoe at the given position in this speed
   * interval.
   */
  general_driving_types get_type_at_position(si::length pos) const;

  /**
   * Determines the difference between the current speed interval transit time
   * and the needed transit time using the updated cruising speed.
   *
   * @remark return == 0: no difference
   * @remark return < 0: new transit time is shorter
   * @remark return > 0: new transit time is longer
   *
   * @param cr_speed_cand the current cruising speed candidate to calculate the
   * transit time for
   *
   * @return the difference between the last full run and the potential change
   * of transit time when using cr_speed_cand.
   */
  si::time get_transit_time_difference_cruising(si::speed cr_speed_cand) const;
};

using speed_intervals = std::vector<speed_interval>;

/**
 * Determines all speed intervals found in a list of interval structs.
 *
 * @remark works only on halt-to-halt interval lists.
 *
 * @param intervals a list of interval structs. They may have different
 * speed_limits.
 * @return a list of speed_interval structs. Every speed interval struct
 * contains all directly connected interval structs with the same speed limit
 */
speed_intervals get_speed_intervals(soro::vector<interval> const& intervals);

std::vector<runtime_result> run_complete(rs::train_physics const& tp,
                                         speed_intervals& speed_intervals,
                                         si::length step_size);

/**
 * Runs a partial simulation starting at speed_interval at position from_idx.
 *
 * @remark works only on halt-to-halt interval lists
 *
 * @remark 0 \<= from_idx \<= len(speed_intervals) - 1
 *
 * @param tp a train defined by a train_physics
 * @param speed_intervals a list of speed intervals
 * @param step_size step size during simulation
 * @param from_idx first speed interval to consider
 *
 * @return a list of runtime_result describing the complete track to be
 * simulated.
 */
std::vector<runtime_result> run_from(rs::train_physics const& tp,
                                     speed_intervals& speed_intervals,
                                     si::length step_size, int from_idx);

}  // namespace soro::runtime
