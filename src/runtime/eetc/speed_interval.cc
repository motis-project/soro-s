#include "soro/runtime/eetc/speed_interval.h"

namespace soro::runtime {

si::length speed_interval::length() const { return this->end_ - this->start_; }

bool speed_interval::has_driving_regime_type(general_driving_types type) const {
  return std::any_of(
      this->driving_regimes_.begin(), this->driving_regimes_.end(),
      [type](const auto& regime) { return regime.type_ == type; });
}

std::tuple<si::speed, si::speed> speed_interval::get_cruising_speed_range(
    bool safe_range) {
  si::speed min_speed;
  auto max_speed = si::speed{INT_MIN};

  // determine min speed
  min_speed = std::get<0>(this->driving_regimes_[0].get_speed_range());

  // if last regime is either coasting or braking, choose as min speed the
  // maximal value of (min speed of acceleration, min speed of last regime
  // (either coasting or braking).
  if (this->has_driving_regime_type(COASTING) ||
      this->has_driving_regime_type(MAX_BRAKING)) {
    auto last_regime_speed_range =
        this->driving_regimes_.back().get_speed_range();
    if (safe_range && min_speed < std::get<0>(last_regime_speed_range)) {
      min_speed = std::get<0>(last_regime_speed_range);
    }
  }

  // determine max speed
  for (auto& regime : this->driving_regimes_) {
    auto regime_speed_range = regime.get_speed_range();
    if (max_speed < std::get<1>(regime_speed_range)) {
      max_speed = std::get<1>(regime_speed_range);
    }
  }

  return std::make_tuple(min_speed, max_speed);
}

std::tuple<si::length, si::length> speed_interval::get_cruising_interval(
    si::speed cr_speed_cand) const {
  // test if acceleration and deceleration in speed_interval
  utl::verify(this->has_driving_regime_type(MAX_ACCELERATION),
              "first regime must be a acceleration regime.");
  utl::verify(this->has_driving_regime_type(COASTING) ||
                  this->has_driving_regime_type(MAX_BRAKING),
              "last regime must be a braking regime.");

  // define no interval found return value
  auto no_interval = std::make_tuple(si::length{-1.0F}, si::length{-1.0F});

  // determine cruising interval start
  auto start = general_driving::intersection_point_with_constant(
      this->driving_regimes_.begin()->last_run_, cr_speed_cand);

  // if start is not found in acceleration: no cruising interval can be found in
  // this speed interval
  if (start == si::length{-1.0F}) {
    return no_interval;
  }

  // determine cruising interval end (intersection point either in coasting (if
  // available) or braking)
  auto end = si::length{-1.0F};
  if (this->has_driving_regime_type(COASTING)) {
    end = general_driving::intersection_point_with_constant(
        (this->driving_regimes_.end() - 2)->last_run_, cr_speed_cand, false);
  }

  if (end == si::length{-1.0F}) {
    end = general_driving::intersection_point_with_constant(
        this->driving_regimes_.back().last_run_, cr_speed_cand, false);
  }

  // if end is not found in either coasting or braking: no cruising interval can
  // be found in this speed interval
  if (end == si::length{-1.0F}) {
    return no_interval;
  }

  // valid cruising interval found
  return std::make_tuple(start, end);
}

si::time speed_interval::get_transit_time() const {
  auto end_time = this->driving_regimes_.back().last_run_.back().time_;
  auto start_time = this->driving_regimes_.begin()->last_run_.begin()->time_;

  return end_time - start_time;
}

si::time speed_interval::get_time_at_position(si::length pos) const {
  // (1) tests whether position is in speed interval or not
  utl::verify(this->start_ <= pos && pos <= this->end_,
              "pos must be a value between interval start and end.");

  // (2) search regime containing position information
  for (auto const& regime : this->driving_regimes_) {
    if (regime.start_ <= pos && pos <= regime.end_) {
      // (3) get last_run_ idx based on regime start and position
      auto idx = static_cast<std::size_t>(pos.val_) -
                 static_cast<std::size_t>(regime.start_.val_);

      // (4) get time at last_run_ idx
      return regime.last_run_[idx].time_;
    }
  }

  // 404 - FALLBACK
  return si::time{-1.0F};
}

si::time speed_interval::get_time_at_end() const {
  return this->get_time_at_position(this->end_);
}

si::speed speed_interval::get_speed_at_position(si::length pos) const {
  // (1) tests whether position is in speed interval or not
  utl::verify(this->start_ <= pos && pos <= this->end_,
              "pos must be a value between interval start and end.");

  // (2) search regime containing position information
  for (auto const& regime : this->driving_regimes_) {
    if (regime.start_ <= pos && pos <= regime.end_) {
      // (3) get last_run_ idx based on regime start and position
      auto idx = static_cast<std::size_t>(pos.val_) -
                 static_cast<std::size_t>(regime.start_.val_);

      // (4) get time at last_run_ idx
      return regime.last_run_[idx].speed_;
    }
  }

  // 404 - FALLBACK
  return si::speed{-1.0F};
}

general_driving_types speed_interval::get_type_at_position(
    si::length pos) const {
  // (1) tests whether position is in speed interval or not
  utl::verify(this->start_ <= pos && pos <= this->end_,
              "pos must be a value between interval start and end.");

  // (2) search regime containing position information
  for (auto const& regime : this->driving_regimes_) {
    if (regime.start_ <= pos && pos <= regime.end_) {
      // (3) get type at last_run_ idx
      return regime.type_;
    }
  }

  return {};
}

si::time speed_interval::get_transit_time_difference_cruising(
    si::speed cr_speed_cand) const {
  // get cruising interval
  auto cr_interval = this->get_cruising_interval(cr_speed_cand);

  // invalid cruising interval: return no difference in time
  // remark: cr_interval is invalid if first and second value is -1.0F
  if (std::get<0>(cr_interval) == si::length{-1.0F}) {
    return si::time{0.0F};
  }

  // known: cr_interval is valid cruising interval
  // get new transit time, combination of pre-cruise, cruise and post-cruise
  auto dt_pre = this->get_time_at_position(std::get<0>(cr_interval)) -
                this->get_time_at_position(this->start_);
  auto dt_post = this->get_time_at_position(this->end_) -
                 this->get_time_at_position(std::get<1>(cr_interval));
  auto dt_cruise =
      (std::get<1>(cr_interval) - std::get<0>(cr_interval)) / cr_speed_cand;

  auto new_transit_time = dt_pre + dt_cruise + dt_post;

  // get current transit time
  auto current_transit_time = this->get_transit_time();

  // return difference
  return new_transit_time - current_transit_time;
}

speed_intervals get_speed_intervals(std::vector<interval> const& intervals) {
  speed_intervals result{};

  // if intervals is empty: no speed interval
  if (intervals.empty()) {
    return {};
  }

  // otherwise: at least two intervals are needed to calculate speed intervals
  utl::verify(intervals.size() >= 2UL,
              "speed_interval::get_speed_intervals: There must be at least two "
              "intervals");

  // initialize first speed interval properties
  std::vector<interval> current_intervals{intervals[0]};
  si::length start = intervals[0].distance_;
  si::length end = intervals[1].distance_;

  // first and last interval have the halt tag enabled
  utl::verify(
      intervals.begin()->halt_,
      "speed_interval::get_speed_intervals: First interval must have a halt.");
  utl::verify(
      intervals.back().halt_,
      "speed_interval::get_speed_intervals: Last interval must have a halt.");

  // WHY size() - 1?
  // The intervals distance property describes the starting point of the
  // interval. Therefore, the last interval is only used to limit the length of
  // the second to last interval. This yields to an irrelevant interval: the
  // last one. However, not declaring it irrelevant results in an interval with
  // unknown length.
  for (std::size_t i = 1UL; i < intervals.size() - 1; ++i) {
    // current interval is part of the current speed interval
    utl::verify(!intervals[i].halt_,
                "speed_interval::get_speed_intervals: No halt allowed between "
                "first and last interval.");
    if (current_intervals[0].speed_limit_ == intervals[i].speed_limit_) {
      // set new speed interval end point; cannot lead to an IndexOutOfBound (i
      // < size - 1)
      end = intervals[i + 1].distance_;
      current_intervals.emplace_back(intervals[i]);
    } else {
      // otherwise: current interval has different speed limit, therefore a new
      // speed interval must start here. First. Save the current one.
      // this one cannot be a halt speed interval (as only the last interval
      // declares a halt
      si::speed const speed_limit = current_intervals[0].speed_limit_;
      bool const halt = false;

      auto const next_speed_interval =
          speed_interval(start, end, speed_limit, halt, current_intervals);

      result.emplace_back(next_speed_interval);

      // initialize next speed interval
      current_intervals = {intervals[i]};
      start = end;
      end = intervals[i + 1].distance_;
    }
  }

  if (!current_intervals.empty()) {
    // add the last (currently) unfinished speed interval
    // the last speed interval has a halt
    si::speed const speed_limit = current_intervals[0].speed_limit_;
    bool const halt = true;

    auto const next_speed_interval =
        speed_interval(start, end, speed_limit, halt, current_intervals);

    result.emplace_back(next_speed_interval);
  }

  return result;
}

std::vector<runtime_result> run_complete(rs::train_physics const& tp,
                                         speed_intervals& speed_intervals,
                                         si::length step_size) {
  std::vector<runtime_result> rr;
  si::time current_time{si::ZERO<si::time>};

  for (auto& speed_int : speed_intervals) {
    ++speed_int.run_ctr_;
    for (auto& regime : speed_int.driving_regimes_) {
      regime.t0_ = current_time;

      if (!rr.empty()) {
        rr.pop_back();
      }
      auto next_rr = regime.run(tp, speed_int.intervals_, regime.vel0_,
                                regime.start_, regime.t0_, step_size);

      rr.insert(rr.end(), next_rr.begin(), next_rr.end());
      current_time = rr.back().time_;
    }
  }
  return rr;
}

std::vector<runtime_result> run_from(rs::train_physics const& tp,
                                     speed_intervals& speed_intervals,
                                     si::length step_size, int from_idx) {
  // verify: 0 <= from idx <= len(speed_intervals) - 1
  utl::verify(
      0 <= from_idx,
      "speed_interval::run_from::from_idx must be greater than or equal to 0.");
  utl::verify(
      from_idx <= static_cast<int>(speed_intervals.size() - 1),
      "speed_interval::run_from::from_idx: idx not found, ArrayOutOfBounds.");

  // call run_complete if from_idx is zero
  if (from_idx == 0) {
    return run_complete(tp, speed_intervals, step_size);
  }

  std::vector<runtime_result> rr;
  std::vector<runtime_result> next_rr;
  si::time current_time{si::ZERO<si::time>};

  int current_idx = 0;

  for (auto& speed_int : speed_intervals) {
    // update counter iff speed_int will be simulated
    if (current_idx >= from_idx) {
      ++speed_int.run_ctr_;
    }

    for (auto& regime : speed_int.driving_regimes_) {
      regime.t0_ = current_time;

      // remove last runtime_result to avoid duplicates
      if (!rr.empty()) {
        rr.pop_back();
      }

      if (!(current_idx >= from_idx)) {
        // do not simulate speed_interval
        next_rr = regime.last_run_;
      } else {
        // simulate speed_interval
        next_rr = regime.run(tp, speed_int.intervals_, regime.vel0_,
                             regime.start_, regime.t0_, step_size);
      }

      // add runtime_results to result vector
      rr.insert(rr.end(), next_rr.begin(), next_rr.end());
      current_time = rr.back().time_;
    }
    // next speed interval -> update current index
    ++current_idx;
  }

  return rr;
}

}  // namespace soro::runtime
