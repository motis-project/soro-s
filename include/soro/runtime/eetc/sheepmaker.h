#pragma once

#include "soro/runtime/driving_regimes/general.h"
#include "soro/runtime/eetc/speed_interval.h"
#include "soro/runtime/interval.h"
#include "soro/runtime/runtime.h"

namespace soro::runtime {

struct sheepmaker_data {
  sheepmaker_data() = delete;
  sheepmaker_data(rs::train_physics tp, interval_list intervals,
                  si::length const step_size)
      : tp_(std::move(tp)),
        intervals_(std::move(intervals)),
        step_size_(step_size) {}
  sheepmaker_data(rs::train_physics tp, interval_list intervals,
                  si::length const step_size, tt::stop_time start,
                  tt::stop_time end)
      : tp_(std::move(tp)),
        intervals_(std::move(intervals)),
        step_size_(step_size),
        start_(start),
        end_(end) {}

  // fixed values
  rs::train_physics tp_;
  interval_list intervals_;
  si::length step_size_;

  // values that may change in time
  speed_intervals speed_intervals_{};
  general_driving_regimes init_driving_regimes_{};
  tt::stop_time start_{};
  tt::stop_time end_{};

  // results
  runtime_results rr_{};

  // functions

  /**
   * @brief Returns the travel time at the end of this container.
   *
   * @remark Uses get_time_at_position(si::length) of the last speed interval to
   * retrieve the travel time at end of this container.
   *
   * @note c.f. include/soro/runtime/eetc/sheepmaker.h
   *
   * @return The (travel) time at the end of this container.
   */
  si::time get_time_at_end() const;
};

struct coasting_candidates {
  double step_size_{};
  double coasting_point_idx_{};

  /**
   * Initializes the coasting_candidates struct.
   * During the initializes a coasting regime at the center of the
   * speed_interval is generated, if there is none.
   *
   * This constructor also calculates the step_size and the coasting_point_idx_
   * attribute
   *
   * @param data a sheepmaker data container storing all relevant data
   * @param speed_interval the speed interval to calculate the optimal coasting
   * point for
   */
  coasting_candidates(sheepmaker_data& data, speed_interval& speed_interval,
                      int run_from_idx = 0);

  void set_next_cp_candidate(sheepmaker_data& data,
                             struct speed_interval& speed_interval,
                             si::time current_travel_time, si::time duration);

  double get_next_cp_candidate();
};

struct coasting_candidates_v2 {
  si::length step_size_{};
  si::length coasting_point_{};

  bool last_step_right{};

  /**
   * Initializes the coasting_candidates struct.
   * During the initializes a coasting regime at the center of the
   * speed_interval is generated, if there is none.
   *
   * This constructor also calculates the step_size and the coasting_point_idx_
   * attribute
   *
   * @param speed_interval the speed interval to calculate the optimal coasting
   * point for
   */
  coasting_candidates_v2(speed_interval& speed_interval);

  si::length get_next_cp_candidate_left();
  si::length get_next_cp_candidate_right();
  si::length repeat_last_step();
};

struct cruising_candidates {
  speed_interval speed_interval_;

  si::time target_time_{};
  si::time current_time_at_end_{};

  cruising_candidates(speed_interval& speed_interval, si::time target_time,
                      si::time current_time_at_end)
      : speed_interval_(speed_interval),
        target_time_(target_time),
        current_time_at_end_(current_time_at_end) {}

  /**
   * @brief Returns the time objective of a train.
   *
   * @remark time_changed_by < 0: train is faster after change.
   * @remark time_changed_by = 0: travel time remains unchanged.
   * @remark time_changed_by > 0: train is slower after change.
   *
   * @remark returns (current - target) squared (since squared time is not
   * implemented, simple time is returned). Reason: Parabola is continuous,
   * absolute value is not.
   *
   * @param time_changed_by Value by which the current travel time was changed
   * by any changes.
   * @return si::time the absolute difference between the current travel time
   * and the target time.
   */
  si::time optimizer_time(si::time time_changed_by) const;

  si::time get_at(si::speed cr_speed_cand) const;
};

/**
 * Runs the sheepmaker algorithm on a complete route.
 *
 * @param intervals a list of interval elements describing the complete route
 * with at least one halt.
 * @param dispo a list of stations to pass or halt on.
 *
 * @remark: intervals last element must be a halt
 * @remark: dispo pass station if min_stop_time_ is zero, otherwise halt
 *
 * a sheepmaker data container, that contains the algorithms result
 */
std::vector<sheepmaker_data> sheepmaker(rs::train_physics const& tp,
                                        std::vector<interval> const& intervals,
                                        tt::train const& dispo,
                                        si::length step_size);

/**
 * Runs the sheepmaker algorithm from halt to the next halt.
 * Method is working in-place on the sheepmaker data container
 *
 * @param data a sheepmaker data container
 * @param threshold if change in time during update is smaller than threshold,
 * stop algorithm step (stop search for better coating point, for example)
 * @param cruise enable update of cruising speed; default: false, currently
 * unvailable
 *
 * @remark data.speed_intervals last speed_interval must be a halt.
 */
void sheepmaker_per_halt(sheepmaker_data& data, float threshold = 0.01F,
                         bool cruise = false);

/**
 * Runs a part of the sheepmaker algorithm to find and set coasting point of the
 * current speed_interval.
 *
 * @param data a sheepmaker container, data is used to simulate a complete run.
 * @param speed_interval the speed interval to update.
 * @param threshold if change of travel time is smaller than the threhsold [0,
 * 1], then a new optimal coasting point is found.
 *
 * @remark method works in place. Therefore it is updating data and the
 * speed_interval
 */
void sheepmaker_update_coasting_point(sheepmaker_data& data,
                                      speed_interval& speed_interval,
                                      float threshold, int run_from_idx = 0);

void sheepmaker_set_coasting_point(sheepmaker_data& data, speed_interval&,
                                   float threshold);

/**
 * @brief Runs a part of the sheepmaker algorithm to find and set the cruising
 * speed of the current speed_interval.
 *
 * @param data a sheepmaker container, used to get general train run
 * information.
 * @param speed_interval  the speed interval to update.
 * @param threshold if change of travel time is smaller than the threshold, then
 * a "optimal" cruising speed is found.
 */
void sheepmaker_update_cruising_speed(sheepmaker_data& data,
                                      speed_interval& speed_interval,
                                      si::speed threshold = si::speed{1.0F});

/**
 * Initialization of the sheepmaker algorithm.
 *
 * @param data a sheepmaker data container
 */
void sheepmaker_initialization(sheepmaker_data& data);

}  // namespace soro::runtime