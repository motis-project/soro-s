#include "soro/runtime/driving_regimes/driving_regime.h"

#include <cmath>

namespace soro::runtime {

driving_regime::~driving_regime() = default;

/**
 * Searches the intersection point of two runtime_result vectors regarding the
 * velocities at all given positions. Returns m{-1.0F} if there is non.
 *
 * Both (given) vectors must consist of at least two runtime_result elements.
 * Both vectors must be of the same size: rr_a.size() == rr_b.size
 * For all runtime_results i it holds rr_a[i].distance == rr_b[i].distance,
 * therefore the runtime_results at position i differ in time and velocity, but
 * not in distance.
 *
 * The search must distinguish four cases:
 * 1st: rr_a[i].speed == rr_b[i].speed => intersection point at rr_a[i].distance
 * = rr_b[i].distance
 *
 * 2nd: rr_a[i + 1].speed == rr_b[i + 1].speed => intersection point at rr_a[i +
 * 1].distance = rr_b[i + 1].distance
 *
 * 3rd: intersection between rr_a[i].distance and rr_a[i + 1].distance.
 * Intersection point is set to rr_a[i].distance. Therefore the intersection
 * point is always a multiple of delta_m.
 *
 * 4th: no intersection regarding the velocities.
 *
 * @param rr_a runtime_results of one driving regime.
 * @param rr_b runtime_results of another driving regime.
 * @return the intersection point of rr_a and rr_b or -1.0F if no intersection
 * was found.
 */
si::length driving_regime::intersection_point(runtime_results const& rr_a,
                                              runtime_results const& rr_b,
                                              bool ignore_border) {

  // rr_a, rr_b sanity checks
  utl::verify(rr_a.size() >= 2 && rr_b.size() >= 2,
              "Both runtime_results must consist of at least two "
              "runtime_result elements.");
  utl::verify(rr_a.begin()->distance_ == rr_b.begin()->distance_,
              "Both runtime_results must start with the same distance value.");
  utl::verify(rr_a.back().distance_ == rr_b.back().distance_,
              "Both runtime_results must end with the same distance value.");
  utl::verify(rr_a.size() == rr_b.size(),
              "Both runtime results must have the same size.");

  auto cutoff = 0UL;
  if (ignore_border) {
    cutoff = 1UL;
  }

  si::length latest_intersection_point = {-1.0F};

  for (std::size_t i = cutoff; i < rr_a.size() - 1; ++i) {
    // sanity checks for runtime_result at i and at i + 1
    utl::verify(rr_a[i].distance_ == rr_b[i].distance_,
                "Distance at i must be the same for rr_a and rr_b. i: " +
                    std::to_string(i));
    utl::verify(rr_a[i + 1].distance_ == rr_b[i + 1].distance_,
                "Distance at i+1 must be the same for rr_a and rr_b. i+1: " +
                    std::to_string(i + 1));

    if (rr_a[i].time_ < si::time{0.0F} || rr_b[i].time_ < si::time{0.0F}) {
      break;
    }

    if (rr_a[i + 1].time_ < si::time{0.0F} ||
        rr_b[i + 1].time_ < si::time{0.0F}) {
      break;
    }

    // is the intersection point at i, i + 1 or between?
    if (rr_a[i].speed_ == rr_b[i].speed_) {
      latest_intersection_point = rr_a[i].distance_;
      continue;
    }

    if (rr_a[i + 1].speed_ == rr_b[i + 1].speed_) {
      latest_intersection_point = rr_a[i + 1].distance_;
      continue;
    }

    auto const delta_vel_at_i = rr_a[i].speed_ - rr_b[i].speed_;
    auto const delta_vel_at_i_1 = rr_a[i + 1].speed_ - rr_b[i + 1].speed_;

    auto const sgn_delta_vel_at_i = std::signbit(delta_vel_at_i.val_);
    auto const sgn_delta_vel_at_i_1 = std::signbit(delta_vel_at_i_1.val_);

    if (sgn_delta_vel_at_i != sgn_delta_vel_at_i_1) {
      latest_intersection_point = rr_a[i].distance_;
    }
  }

  // no intersection point between rr_a and rr_b found
  return latest_intersection_point;
}

si::length driving_regime::intersection_point_with_constant(
    runtime_results const& rr_a, si::speed const& speed,
    bool const& search_first) {
  auto cutoff = 0UL;

  si::length latest_intersection_point = {-1.0F};

  for (std::size_t i = cutoff; i < rr_a.size() - 1; ++i) {
    if (rr_a[i].speed_ == speed) {
      latest_intersection_point = rr_a[i].distance_;
      if (search_first) {
        return latest_intersection_point;
      } else {
        continue;
      }
    }

    if (rr_a[i + 1].speed_ == speed) {
      latest_intersection_point = rr_a[i + 1].distance_;
      if (search_first) {
        return latest_intersection_point;
      } else {
        continue;
      }
    }

    auto const delta_vel_at_i = rr_a[i].speed_ - speed;
    auto const delta_vel_at_i_1 = rr_a[i + 1].speed_ - speed;

    auto const sgn_delta_vel_at_i = std::signbit(delta_vel_at_i.val_);
    auto const sgn_delta_vel_at_i_1 = std::signbit(delta_vel_at_i_1.val_);

    if (sgn_delta_vel_at_i != sgn_delta_vel_at_i_1) {
      latest_intersection_point = rr_a[i].distance_;
      if (search_first) {
        return latest_intersection_point;
      } else {
        continue;
      }
    }
  }

  if (!search_first && rr_a.back().speed_ == speed) {
    return rr_a.back().distance_;
  }

  return latest_intersection_point;
}

std::vector<runtime_result> driving_regime::simulate(
    rs::train_physics const& tp, interval_list const& intervals,
    si::length const start, si::length const end, si::speed const velocity,
    si::length const offset, si::time const time_offset,
    si::length const step_size, bool const reverse) {
  if (reverse) {
    // interpretation of velocity as velocity at end
    return this->simulate_reverse_on(tp, intervals, start, end, velocity,
                                     offset, time_offset, step_size);
  }
  // interpretation of velocity as velocity at start
  return this->simulate_on(tp, intervals, start, end, velocity, offset,
                           time_offset, step_size);
}

}  // namespace soro::runtime
