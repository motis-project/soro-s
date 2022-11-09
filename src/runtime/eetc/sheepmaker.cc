#include "soro/runtime/eetc/sheepmaker.h"

#include <cmath>

#include "soro/runtime/driving_regimes/driving_regime.h"
#include "soro/runtime/driving_regimes/general.h"
#include "soro/runtime/eetc/golden_section_search.h"

namespace soro::runtime {

si::time sheepmaker_data::get_time_at_end() const {
  return this->speed_intervals_.back().get_time_at_end();
}

std::vector<sheepmaker_data> sheepmaker(rs::train_physics const& tp,
                                        interval_list const& intervals,
                                        tt::train const& dispo,
                                        si::length step_size) {
  utl::verify(intervals[0].halt_, "First interval must be a halt.");
  utl::verify(intervals.end()[-2].halt_,
              "Second to last interval must be a halt.");

  utl::verify(dispo.stop_times_.front().is_halt(),
              "First dispo entry must be a halt.");
  utl::verify(dispo.stop_times_.back().is_halt(),
              "Last dispo entry must be a halt.");

  utl::verify(intervals.size() >= 2,
              "sheepmaker: there must be at least two separate intervals.");
  utl::verify(dispo.stop_times_.size() >= 2,
              "sheepmaker: there must be at least to separate dispo entries.");

  std::vector<sheepmaker_data> halt_to_halt_containers{};

  auto intervals_current_start = 0UL;
  auto intervals_current_end = 1UL;

  auto dispo_current_start = 0UL;
  auto dispo_current_end = 1UL;

  while (intervals_current_start < intervals.size() - 2 &&
         dispo_current_start < dispo.stop_times_.size() - 1) {
    soro::vector<interval> halt_to_halt_intervals{};

    auto next_interval = intervals[intervals_current_start];
    // use *.val_ since distance/distance is of type fraction not double
    next_interval.distance_ =
        step_size * std::round((next_interval.distance_ / step_size).val_);

    halt_to_halt_intervals.emplace_back(next_interval);
    for (std::size_t i = intervals_current_start + 1UL; i < intervals.size();
         ++i) {
      if (intervals[i].halt_) {
        intervals_current_end = i;

        next_interval = intervals[i];
        // use *.val_ since distance/distance is of type fraction not double
        next_interval.distance_ =
            step_size * std::round((next_interval.distance_ / step_size).val_);

        halt_to_halt_intervals.emplace_back(next_interval);
        break;
      }
      next_interval = intervals[i];
      // use *.val_ since distance/distance is of type fraction not double
      next_interval.distance_ =
          step_size * std::round((next_interval.distance_ / step_size).val_);

      halt_to_halt_intervals.emplace_back(next_interval);
    }

    for (auto i = dispo_current_start + 1UL; i < dispo.stop_times_.size();
         ++i) {
      if (dispo.stop_times_[i].is_halt()) {
        dispo_current_end = i;
        break;
      }
    }

    auto halt_to_halt_container =
        sheepmaker_data(tp, halt_to_halt_intervals, step_size,
                        dispo.stop_times_[dispo_current_start],
                        dispo.stop_times_[dispo_current_end]);

    sheepmaker_initialization(halt_to_halt_container);
    sheepmaker_per_halt(halt_to_halt_container);

    halt_to_halt_containers.emplace_back(halt_to_halt_container);

    intervals_current_start = intervals_current_end;
    dispo_current_start = dispo_current_end;
  }

  return halt_to_halt_containers;
}

/**
 * Runs the sheepmaker algorithm from halt to the next halt.
 * Methods is working in-place on the sheepmaker data container
 *
 * @remark: update cruising speed is not implemented yet
 *
 * Searches for speed_intervals with a braking/deceleration driving regime
 * and tries to optimize the (newly created) or existing coating point of
 * the speed_interval to achieve a better arrival time (better: as close to
 * the timetable arrival time as possible)
 *
 * @param data a sheepmaker data container
 * @param threshold update threshold. stop algorithm step, if change in time
 * is less than threshold (percentage value between zero and one); default:
 * 0.01
 * @param cruise true if cruising speed should be changed to achieve optimal
 * arrival time, otherwise false; default false
 */
void sheepmaker_per_halt(sheepmaker_data& data, float threshold, bool cruise) {
  utl::verify(data.speed_intervals_.back().halt_,
              "last speed interval must be a halt");
  utl::verify(!cruise, "update cruising is currently not implemented");

  for (auto& speed_interval : data.speed_intervals_) {
    if (speed_interval.driving_regimes_.back().type_ == MAX_BRAKING) {
      if (!speed_interval.has_driving_regime_type(COASTING)) {
        sheepmaker_set_coasting_point(data, speed_interval, threshold);
      }

      sheepmaker_update_cruising_speed(data, speed_interval);
    }
  }
};

/**
 * Initialization of the Sheepmaker-Algorithm data container.
 *
 * Before. data has a train physics, a list of intervals from a single halt
 * to the next halt and a step_size
 *
 * After. Intervals are sorted into speed_intervals. For every speed_interval a
 * ACCELERATION is added. In Case of BRAKING-Events a default BRAKING regime is
 * added as well. Therefore this method initializes the following data container
 * parameters: speed_intervals_, driving_regimes, rr_
 *
 * Remark. speed_intervals gather all corresponding driving regimes into one
 * speed interval. However a list of driving_regimes is generated too. *
 *
 * @param data the sheepmaker data container to work on in-place
 */
void sheepmaker_initialization(sheepmaker_data& data) {
  data.speed_intervals_ = get_speed_intervals(data.intervals_);

  si::time current_time{si::ZERO<si::time>};
  si::speed current_velocity{si::ZERO<si::speed>};

  general_driving_regimes init_regimes = {};
  for (std::size_t i = 0UL; i < data.speed_intervals_.size(); ++i) {
    speed_interval& speed_int = data.speed_intervals_[i];

    // every speed_interval needs an acceleration regime
    auto accelerate =
        general_driving(speed_int.start_, speed_int.end_, current_velocity,
                        current_time, MAX_ACCELERATION);
    auto rr_accelerate = accelerate.simulate(
        data.tp_, speed_int.intervals_, accelerate.start_, accelerate.end_,
        current_velocity, accelerate.start_, current_time, data.step_size_,
        false);

    if (i == data.speed_intervals_.size() - 1 ||
        data.speed_intervals_[i].speed_limit_ >
            data.speed_intervals_[i + 1].speed_limit_) {
      // there must be a deceleration regime in this speed_interval
      auto decelerate =
          general_driving(speed_int.start_, speed_int.end_, current_velocity,
                          current_time, MAX_BRAKING);

      // determine deceleration velocity target at decelerate.end
      si::speed target_velocity{si::ZERO<si::speed>};
      if (!speed_int.halt_) {
        target_velocity = data.speed_intervals_[i + 1].speed_limit_;
      }
      auto rr_decelerate = decelerate.simulate(
          data.tp_, speed_int.intervals_, speed_int.start_, speed_int.end_,
          target_velocity, accelerate.start_, current_time, data.step_size_,
          true);

      // determine intersection point of accelerate and decelerate
      auto i_point =
          driving_regime::intersection_point(rr_accelerate, rr_decelerate);
      double const ip_idx =
          ((i_point - accelerate.start_) / data.step_size_).val_;

      // update regimes using intersection point
      accelerate.end_ = i_point;
      decelerate.start_ = i_point;
      decelerate.t0_ = rr_accelerate[ip_idx].time_;
      decelerate.vel0_ = target_velocity;

      // update time and velocity
      // current_time += rr_decelerate.back().time_ -
      // rr_decelerate[ip_idx].time_ +
      //                 rr_accelerate[ip_idx].time_;
      // current_velocity = target_velocity;

      rr_accelerate = accelerate.simulate(
          data.tp_, speed_int.intervals_, accelerate.start_,
          accelerate.end_ - data.step_size_, current_velocity,
          accelerate.start_, current_time, data.step_size_, false);
      current_time = rr_accelerate.back().time_;
      rr_decelerate = decelerate.simulate(data.tp_, speed_int.intervals_,
                                          decelerate.start_, decelerate.end_,
                                          decelerate.vel0_, decelerate.start_,
                                          current_time, data.step_size_, true);
      current_time = rr_decelerate.back().time_;
      current_velocity = rr_decelerate.back().speed_;

      // add both to the vector of driving regimes
      init_regimes.emplace_back(accelerate);
      init_regimes.emplace_back(decelerate);
      speed_int.driving_regimes_.emplace_back(accelerate);
      speed_int.driving_regimes_.emplace_back(decelerate);

      continue;
    }

    // update time and velocity
    current_time = rr_accelerate.back().time_;
    current_velocity = rr_accelerate.back().speed_;

    init_regimes.emplace_back(accelerate);
    speed_int.driving_regimes_.emplace_back(accelerate);
  }

  data.init_driving_regimes_ = init_regimes;
  data.rr_ = run_from(data.tp_, data.speed_intervals_, data.step_size_, 0);
}

void sheepmaker_set_coasting_point(sheepmaker_data& data,
                                   speed_interval& speed_interval,
                                   float threshold) {
  utl::verify(0.0F <= threshold && threshold <= 1.0F,
              "Threshold must be a real number between 0F and 1F.");

  si::time const target_duration{
      static_cast<float>(data.end_.arrival_ - data.start_.departure_)};
  si::time const current_travel_time{data.rr_.back().time_ -
                                     data.rr_.begin()->time_};

  auto last_travel_time = current_travel_time;

  bool found_first_cp = false;
  bool finished = false;

  // initialize coasting point candidates struct
  auto candidates = coasting_candidates_v2(speed_interval);

  auto last_coasting_candidate = candidates.coasting_point_;
  si::length last_intersection_point{};
  runtime_results last_rr_coasting{};
  runtime_results last_rr_deceleration{};

  utl::verify(
      candidates.coasting_point_ < speed_interval.driving_regimes_[0].end_,
      "Initial Coasting Point must be in ACCELERATION.");

  auto coasting = general_driving({NAN}, {NAN}, {NAN}, {NAN}, COASTING);
  auto deceleration = speed_interval.driving_regimes_.back();

  while (!finished) {
    coasting.start_ = candidates.coasting_point_;
    coasting.t0_ =
        speed_interval.get_time_at_position(candidates.coasting_point_);
    coasting.vel0_ =
        speed_interval.get_speed_at_position(candidates.coasting_point_);

    auto rr_coasting = coasting.simulate(
        data.tp_, speed_interval.intervals_, coasting.start_, deceleration.end_,
        coasting.vel0_, coasting.start_, coasting.t0_, data.step_size_, false);
    auto rr_deceleration = deceleration.simulate(
        data.tp_, speed_interval.intervals_, coasting.start_, deceleration.end_,
        deceleration.vel0_, deceleration.start_, coasting.t0_, data.step_size_,
        true);

    auto intersection_point =
        driving_regime::intersection_point(rr_coasting, rr_deceleration, true);

    if (intersection_point == si::length{-1.0F}) {
      if (candidates.step_size_ <= si::length{1.0F}) {
        finished = true;
        continue;
      }

      if (!found_first_cp) {
        if (speed_interval.get_type_at_position(candidates.coasting_point_) ==
            MAX_ACCELERATION) {
          candidates.get_next_cp_candidate_right();
        } else {
          candidates.get_next_cp_candidate_left();
        }
      } else {
        candidates.coasting_point_ = last_coasting_candidate;
        candidates.repeat_last_step();
      }

      continue;
    }
    found_first_cp = true;
    last_coasting_candidate = candidates.coasting_point_;
    last_intersection_point = intersection_point;
    last_rr_coasting = rr_coasting;
    last_rr_deceleration = rr_deceleration;

    // KNOWN: intersection point found, therefore, current coasting point may be
    // "the one"
    auto time_in_acceleration =
        speed_interval.get_time_at_position(candidates.coasting_point_) -
        speed_interval.get_time_at_position(speed_interval.start_);

    double const intersection_point_idx{
        ((intersection_point - coasting.start_) / data.step_size_).val_};

    auto time_in_coasting =
        rr_coasting[intersection_point_idx].time_ - rr_coasting.begin()->time_;
    auto time_in_deceleration = rr_deceleration.back().time_ -
                                rr_deceleration[intersection_point_idx].time_;

    auto next_travel_time =
        time_in_acceleration + time_in_coasting + time_in_deceleration;

    // if next and last in threshold: finished or next_tr == target_duration ->
    // update regimes!! + simulate
    double const change{std::abs((next_travel_time - last_travel_time).val_) /
                        last_travel_time.val_};

    if (next_travel_time == target_duration || change <= threshold) {
      finished = true;
      continue;
    }

    // else: check for left or right step
    if (next_travel_time < target_duration) {
      // next_travel_time < target_duration
      // therefore, the train is too fast => move coasting point to the right
      candidates.get_next_cp_candidate_left();
    } else {
      // next_travel_time > target_duration
      // therefore, the train is too slow => move coasting point to the right
      candidates.get_next_cp_candidate_right();
    }

    last_travel_time = next_travel_time;
  }
  // UPDATE DRIVING REGIMES AFTER CALCULATION OF VALID COASTING POINT
  // UPDATE IFF AT LEAST ONE VALID CP HAS BEEN FOUND
  if (found_first_cp) {
    std::cout << "found coasting point: " << last_coasting_candidate
              << std::endl;
    // calculate intersection_point
    double const intersection_point_idx =
        ((last_intersection_point - coasting.start_) / data.step_size_).val_;

    // update acceleration
    speed_interval.driving_regimes_.begin()->end_ = last_coasting_candidate;

    // update coasting
    coasting.start_ = last_coasting_candidate;
    coasting.end_ = last_intersection_point;
    coasting.vel0_ = last_rr_coasting[intersection_point_idx].speed_;

    // update deceleration
    speed_interval.driving_regimes_.back().start_ = last_intersection_point;

    speed_interval.driving_regimes_.insert(
        speed_interval.driving_regimes_.begin() + 1, coasting);

    data.rr_ = run_complete(data.tp_, data.speed_intervals_, data.step_size_);
  }
}

void sheepmaker_update_coasting_point(sheepmaker_data& data,
                                      speed_interval& speed_interval,
                                      float const threshold, int run_from_idx) {
  std::cout << "sheepmaker_update_coasting_point: MARKED AS DEPRECATED SOON"
            << std::endl;
  utl::verify(0.0F <= threshold && threshold <= 1.0F,
              "Threshold must be a real number between 0F and 1F");

  // determine target travel time (duration) and actual_travel_time
  // (travel_time)
  si::time const duration{
      static_cast<float>(data.end_.arrival_ - data.start_.departure_)};
  si::time travel_time = data.rr_.back().time_ - data.rr_.begin()->time_;

  // is there a coasting driving regime in the speed interval
  // NO:  coasting_candidates initializes the first one, which must be evaluated
  //      before testing the next one
  // YES: set a new coasting point, test performance change with new c.point
  bool const has_coasting =
      speed_interval.driving_regimes_.end()[-2].type_ == COASTING;

  // initialize coasting point candidates struct
  auto candidates = coasting_candidates(data, speed_interval, run_from_idx);

  if (has_coasting) {
    // there is/was an coasting regime in the speed interval
    // set and test travel time with new coasting point
    // get and set first candidate
    candidates.set_next_cp_candidate(data, speed_interval, travel_time,
                                     duration);

    // evaluate simulation with the changed/updated coasting point
    data.rr_ = run_from(data.tp_, data.speed_intervals_, data.step_size_,
                        run_from_idx);
  }

  auto updated_travel_time = data.rr_.back().time_ - data.rr_.begin()->time_;

  // use the floating value, not the SI-value, since
  // no float abs definition for second
  // the change in [0, 1] of travel time after coasting point update
  double change =
      std::abs((updated_travel_time - travel_time).val_) / travel_time.val_;
  // as long as the update is greater than a given threshold: repeat update
  // coasting point procedure
  while (change > threshold) {
    travel_time = updated_travel_time;

    candidates.set_next_cp_candidate(data, speed_interval, travel_time,
                                     duration);
    data.rr_ = run_from(data.tp_, data.speed_intervals_, data.step_size_,
                        run_from_idx);

    updated_travel_time = data.rr_.back().time_ - data.rr_.begin()->time_;
    change =
        std::abs((updated_travel_time - travel_time).val_) / travel_time.val_;
  }
}

void coasting_candidates::set_next_cp_candidate(
    sheepmaker_data& data, struct speed_interval& speed_interval,
    si::time current_travel_time, si::time duration) {
  if (current_travel_time == duration) {
    return;
  }

  general_driving& coasting = speed_interval.driving_regimes_.end()[-2];
  general_driving& deceleration = speed_interval.driving_regimes_.back();
  bool move_right = false;

  if (current_travel_time > duration) {
    move_right = true;
    // move coasting point to the right
    this->coasting_point_idx_ += step_size_;
    coasting.start_ = data.rr_[this->coasting_point_idx_].distance_;
    // either update accelerate or cruising end point
    speed_interval.driving_regimes_.end()[-3].end_ =
        data.rr_[this->coasting_point_idx_].distance_;

    // determine velocity and time at coasting point, will be sued to calculate
    // intersection point of coasting and deceleration
    // runs either acceleration or cruising
    auto temp = speed_interval.driving_regimes_.end()[-3].run(
        data.tp_, speed_interval.intervals_,
        speed_interval.driving_regimes_.end()[-3].vel0_,
        speed_interval.driving_regimes_.begin()->start_,
        speed_interval.driving_regimes_.begin()->t0_, data.step_size_);

    coasting.vel0_ = temp.back().speed_;
    coasting.t0_ = temp.back().time_;

  } else {  // current_travel_time < duration
    // move coasting point left
    this->coasting_point_idx_ -= step_size_;

    // velocity and time is known, since all simulation before the new
    // coasting point remains unchanged
    coasting.start_ = data.rr_[this->coasting_point_idx_].distance_;
    coasting.vel0_ = data.rr_[this->coasting_point_idx_].speed_;
    coasting.t0_ = data.rr_[this->coasting_point_idx_].time_;

    // has cruising: coasting_point is only allowed to be between
    // cruising.start_ and decelerate.end_ !has cruising: coasting point is only
    // allowed to be between accelerate.start_ and decelerate.end_ constraint
    // should be automatically fulfilled by step_size
    speed_interval.driving_regimes_.end()[-3].end_ = coasting.start_;
  }  // endif: train slower/faster than it should be

  // intersection point sets new braking start and coasting end (incl.
  // coasting vel0_)
  auto rr_coasting = coasting.simulate(
      data.tp_, speed_interval.intervals_, coasting.start_, deceleration.end_,
      coasting.vel0_, coasting.start_, coasting.t0_, data.step_size_, false);
  auto rr_deceleration = speed_interval.driving_regimes_.back().simulate(
      data.tp_, speed_interval.intervals_, coasting.start_, deceleration.end_,
      deceleration.vel0_, deceleration.start_, coasting.t0_, data.step_size_,
      true);

  auto intersection_point =
      driving_regime::intersection_point(rr_coasting, rr_deceleration);

  // while "no intersection has been found" and as long as the coasting point is
  // between (accelerate|coasting).start and deceleration.start_ move coasting
  // point stepwise right or left
  // step_wise right if coasting point moved left
  // step_wise left if coasting point moved right
  while (intersection_point == si::length{-1.0F} &&
         coasting.start_ <= deceleration.start_ &&
         coasting.start_ >= speed_interval.driving_regimes_.end()[-3].start_) {
    if (move_right) {
      this->coasting_point_idx_--;
    } else {
      this->coasting_point_idx_++;
    }
    coasting.start_ = data.rr_[this->coasting_point_idx_].distance_;
    coasting.vel0_ = data.rr_[this->coasting_point_idx_].speed_;
    coasting.t0_ = data.rr_[this->coasting_point_idx_].time_;
    speed_interval.driving_regimes_.end()[-3].end_ = coasting.start_;

    // intersection point sets new braking start and coasting end (incl.
    // coasting vel0_)
    rr_coasting = coasting.simulate(
        data.tp_, speed_interval.intervals_, coasting.start_, deceleration.end_,
        coasting.vel0_, coasting.start_, coasting.t0_, data.step_size_, false);
    rr_deceleration = speed_interval.driving_regimes_.back().simulate(
        data.tp_, speed_interval.intervals_, coasting.start_, deceleration.end_,
        deceleration.vel0_, deceleration.start_, coasting.t0_, data.step_size_,
        true);

    intersection_point =
        driving_regime::intersection_point(rr_coasting, rr_deceleration);
  }

  double intersection_point_idx{};
  if (intersection_point != si::length{-1.0F}) {
    intersection_point_idx =
        ((intersection_point - coasting.start_) / data.step_size_).val_;
  } else {
    intersection_point_idx = 0.0F;
    intersection_point = coasting.start_;
  }

  coasting.end_ = intersection_point;
  coasting.vel0_ = rr_deceleration[intersection_point_idx].speed_;
  deceleration.start_ = intersection_point;
  deceleration.t0_ = rr_deceleration[intersection_point_idx].time_;

  this->step_size_ = std::floor(this->step_size_ / 2);
}

coasting_candidates_v2::coasting_candidates_v2(speed_interval& speed_interval) {
  // Requirements: No COASTING, No CRUISING
  si::length const interval_length{speed_interval.end_ - speed_interval.start_};

  // set initial step_size_ (1/2 speed_interval length)
  this->step_size_ = {std::floor(0.5F * interval_length.val_)};

  // set initial coasting_point_ center of speed_interval
  this->coasting_point_ = speed_interval.start_ + this->step_size_;
}

si::length coasting_candidates_v2::get_next_cp_candidate_left() {
  this->last_step_right = false;
  this->step_size_ = {std::floor(0.5 * this->step_size_.val_)};
  this->coasting_point_ = this->coasting_point_ - this->step_size_;
  return this->coasting_point_;
}

si::length coasting_candidates_v2::get_next_cp_candidate_right() {
  this->last_step_right = true;
  this->step_size_ = {std::floor(0.5 * this->step_size_.val_)};
  this->coasting_point_ = this->coasting_point_ + this->step_size_;
  return this->coasting_point_;
}

si::length coasting_candidates_v2::repeat_last_step() {
  if (this->last_step_right) {
    return this->get_next_cp_candidate_right();
  }
  return this->get_next_cp_candidate_left();
}

coasting_candidates::coasting_candidates(sheepmaker_data& data,
                                         speed_interval& speed_interval,
                                         int run_from_idx) {
  // if second to last driving regime is coasting; save as coasting; otherwise:
  // generate coasting regime
  auto coasting = speed_interval.driving_regimes_.end()[-2];
  auto has_coasting = coasting.type_ == COASTING;
  auto has_cruising =
      (speed_interval.driving_regimes_.begin() + 1)->type_ == CRUISING;

  if (!has_coasting) {
    // no coasting: set coasting point at the center of the speed_interval (no
    // cruising, in case of cruising: center if cruising_start and
    // decelerate_end round off to next integer -> convert it back to float to
    // fulfill m type constraint
    si::length coasting_point{};

    // if speed interval has cruising regime: set coasting point between
    // coasting start and speed_interval end
    // otherwise: set coasting point between speed_interval start and end
    if (has_cruising) {
      auto cruising = speed_interval.driving_regimes_[1];
      coasting_point =
          si::length{data.step_size_ *
                     std::round((speed_interval.end_ + cruising.start_).val_ /
                                (2 * data.step_size_).val_)};
    } else {
      coasting_point = si::length{
          data.step_size_ *
          std::round((speed_interval.end_ + speed_interval.start_).val_ /
                     (2 * data.step_size_).val_)};
    }

    // calculate coasting point idx in runtime_results
    this->coasting_point_idx_ =
        ((coasting_point - data.rr_.begin()->distance_) / data.step_size_).val_;
    si::speed const coasting_start_vel =
        data.rr_[this->coasting_point_idx_].speed_;
    si::time const coasting_start_time =
        data.rr_[this->coasting_point_idx_].time_;

    coasting = general_driving(coasting_point, {NAN}, {NAN},
                               coasting_start_time, COASTING);
    auto& deceleration = speed_interval.driving_regimes_.back();

    // determine end point of the coasting regime (intersection point of
    // coasting and deceleration)
    auto rr_deceleration = deceleration.simulate(
        data.tp_, speed_interval.intervals_, coasting.start_, deceleration.end_,
        deceleration.vel0_, coasting.start_, coasting.t0_, data.step_size_,
        true);
    auto rr_coasting = coasting.simulate(data.tp_, speed_interval.intervals_,
                                         coasting.start_, deceleration.end_,
                                         coasting_start_vel, coasting.start_,
                                         coasting.t0_, data.step_size_, false);

    // calculate intersection point and index regarding rr_coasting and
    // rr_deceleration
    auto intersection_point =
        driving_regime::intersection_point(rr_coasting, rr_deceleration);

    /** as long as there is no intersection point search a first point by moving
     * the intersection point to the right
     *
     * USECASE: Maybe the center of the speed interval
     * is not a "good" coasting point candidate. This may be the case when
     * coasting and deceleration have no intersection point. In such a case the
     * coasting point will be moved step_size-wise to the right to find a first
     * coasting point candidate.
     * First candidate: center of speed interval
     * Last candidate: start of deceleration
     */
    while (intersection_point == si::length{-1.0F} &&
           coasting.start_ < deceleration.start_) {

      // determine next intersection point candidate
      this->coasting_point_idx_++;
      coasting.start_ = data.rr_[this->coasting_point_idx_].distance_;
      coasting.vel0_ = data.rr_[this->coasting_point_idx_].speed_;
      coasting.t0_ = data.rr_[this->coasting_point_idx_].time_;

      // determine end point of the coasting regime (intersection point of
      // coasting and deceleration)
      rr_deceleration = deceleration.simulate(
          data.tp_, speed_interval.intervals_, coasting.start_,
          deceleration.end_, deceleration.vel0_, coasting.start_, coasting.t0_,
          data.step_size_, true);
      rr_coasting = coasting.simulate(data.tp_, speed_interval.intervals_,
                                      coasting.start_, deceleration.end_,
                                      coasting.vel0_, coasting.start_,
                                      coasting.t0_, data.step_size_, false);

      // calculate intersection point
      intersection_point =
          driving_regime::intersection_point(rr_coasting, rr_deceleration);
    }

    // if there is an intersection point, then calculate the intersection point
    // index, otherwise: set intersection point and idx to coasting point + 0
    double intersection_point_idx{};
    if (intersection_point != si::length{-1.0F}) {
      intersection_point_idx =
          ((intersection_point - coasting.start_) / data.step_size_).val_;
    } else {
      intersection_point_idx = 0.0F;
      intersection_point = coasting_point;
    }

    coasting.end_ = intersection_point;
    coasting.vel0_ = rr_coasting[intersection_point_idx].speed_;
    deceleration.start_ = intersection_point;
    deceleration.t0_ = rr_coasting[intersection_point_idx].time_;

    speed_interval.driving_regimes_.insert(
        speed_interval.driving_regimes_.end() - 1, coasting);

    if (has_cruising) {
      // coasting point is in cruising regime
      // update cruising regime end point
      speed_interval.driving_regimes_[1].end_ = coasting.start_;
    } else {
      // coasting point is in accelerate regime
      // update accelerate regime end point
      speed_interval.driving_regimes_[0].end_ = coasting.start_;
    }

    data.rr_ = run_from(data.tp_, data.speed_intervals_, data.step_size_,
                        run_from_idx);
  } else {
    this->coasting_point_idx_ =
        ((coasting.start_ - data.rr_.begin()->distance_) / data.step_size_)
            .val_;
  }

  si::time const duration{
      static_cast<float>(data.end_.arrival_ - data.start_.departure_)};
  si::time const current_travel_time =
      data.rr_.back().time_ - data.rr_.begin()->time_;

  if (current_travel_time > duration) {
    // train is slower than it should be. Move coasting point to the right
    // Calculate step_size for a move right: center of deceleration.end_ and
    // coasting.start_
    auto deceleration_end_idx = ((speed_interval.driving_regimes_.back().end_ -
                                  data.rr_.begin()->distance_) /
                                 data.step_size_)
                                    .val_;
    this->step_size_ =
        std::floor((deceleration_end_idx - this->coasting_point_idx_) / 2);
  } else {
    // train is faster than it should be. Move coasting point to the left
    // Calculate step_size for a move left: center of accelerate.start_ and
    // coasting.start
    double start_idx{};
    if (has_cruising) {
      start_idx = ((speed_interval.driving_regimes_[1].start_ -
                    data.rr_.begin()->distance_) /
                   data.step_size_)
                      .val_;
    } else {
      start_idx = ((speed_interval.driving_regimes_.begin()->start_ -
                    data.rr_.begin()->distance_) /
                   data.step_size_)
                      .val_;
    }

    this->step_size_ = std::floor((this->coasting_point_idx_ - start_idx) / 2);
  }
}

// CRUISING SPEED UPDATE METHODS AND FUNCTIONS

si::time cruising_candidates::optimizer_time(si::time time_changed_by) const {
  // si::time target_time{
  //     static_cast<float>(this->end_.arrival_ - this->start_.departure_)};
  auto current_time = this->current_time_at_end_ + time_changed_by;
  return si::time{fabs((current_time - this->target_time_).val_)};
  //(current_time - this->target_time_).val_};
}

si::time cruising_candidates::get_at(si::speed cr_speed_cand) const {
  auto time_difference =
      this->speed_interval_.get_transit_time_difference_cruising(cr_speed_cand);
  return this->optimizer_time(time_difference);
}

void sheepmaker_update_cruising_speed(sheepmaker_data& data,
                                      speed_interval& speed_interval,
                                      si::speed threshold) {
  // (1) get cruising speed interval. Use safe_search.
  auto cr_speed_range = speed_interval.get_cruising_speed_range(true);
  si::speed cr_speed = std::get<0>(cr_speed_range);

  si::time const target_time{
      static_cast<float>(data.end_.arrival_ - data.start_.departure_)};

  // (2) start < end? Y: Run GSS
  if (std::get<0>(cr_speed_range) != std::get<1>(cr_speed_range)) {

    // (4, 5) run golden section search
    auto cr_cand = cruising_candidates(speed_interval, target_time,
                                       data.get_time_at_end());
    cr_speed = golden_section_search_min_optimizer(std::get<0>(cr_speed_range),
                                                   std::get<1>(cr_speed_range),
                                                   threshold, cr_cand);
  }

  // (6) get cruising interval
  auto cr_interval = speed_interval.get_cruising_interval(cr_speed);

  // (7) Check for Safe Insert
  auto start = std::get<0>(cr_interval);
  auto end = std::get<1>(cr_interval);
  auto time_at_start = speed_interval.get_time_at_position(start);
  auto cruising =
      general_driving{start, end, cr_speed, time_at_start, CRUISING};
  auto& braking = speed_interval.driving_regimes_.back();

  // (7.1) SET ACC.END to CRUISING.START
  speed_interval.driving_regimes_.begin()->end_ = start;

  if (speed_interval.has_driving_regime_type(COASTING)) {
    auto& coasting = speed_interval.driving_regimes_.end()[-2];
    if (coasting.start_ <= end && end <= coasting.end_) {
      // (8) UPDATE COASTING START POINT TO CRUISING END
      coasting.start_ = cruising.end_;
    } else if (braking.start_ <= end && end <= braking.end_) {
      // (9) REMOVE COASTING; UPDATE BRAKING START POINT TO CRUISING END
      speed_interval.driving_regimes_.erase(
          speed_interval.driving_regimes_.end() - 2);

      braking.start_ = cruising.end_;
    }
  } else {
    // (10) UPDATE BRAKING START POINT TO CRUISING END
    braking.start_ = cruising.end_;
    braking.vel0_ = cr_speed;
  }

  // (11) INSERT CRUISING REGIME
  speed_interval.driving_regimes_.insert(
      speed_interval.driving_regimes_.begin() + 1, cruising);

  // (12) SIMULATE COMPLETE
  data.rr_ = run_complete(data.tp_, data.speed_intervals_, data.step_size_);
}

}  // namespace soro::runtime
