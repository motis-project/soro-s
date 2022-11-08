#pragma once

#include <vector>

#include "soro/rolling_stock/train_series.h"

#include "soro/runtime/interval.h"
#include "soro/runtime/runtime_physics.h"

namespace soro::runtime {

/**
 * Class description of a driving regime/instruction.
 *
 * A regime is valid from a start to an end point. This local limitation allows
 * a direct assignment of one or more intervals describing the route/line
 * properties.
 *
 */
class driving_regime {

private:
  virtual std::vector<runtime_result> simulate_on(
      rs::train_physics const& tv, std::vector<interval> const& intervals,
      si::length start, si::length end, si::speed vel_start, si::length offset,
      si::time time_offset, si::length step_size) = 0;
  virtual std::vector<runtime_result> simulate_reverse_on(
      rs::train_physics const& tv, std::vector<interval> const& intervals,
      si::length start, si::length end, si::speed vel_end, si::length offset,
      si::time time_offset, si::length step_size) = 0;

public:
  si::length start_, end_;
  si::speed vel0_;
  si::time t0_;

  virtual ~driving_regime();

  driving_regime(const driving_regime&) = default;
  driving_regime(driving_regime&&) = default;
  driving_regime& operator=(const driving_regime&) = default;
  driving_regime& operator=(driving_regime&&) = default;

  driving_regime(si::length const start, si::length const end,
                 si::speed const vel0, si::time const t0)
      : start_(start), end_(end), vel0_(vel0), t0_(t0) {}

  // void run(train_variant const& tv);

  /**
   * Calculates the intersection point of two driving regimes regarding the
   * velocity. Both runtime_results (rr_a and rr_b) must start and end at the
   * same position
   *
   * @param rr_a runtime_results of one driving regime.
   * @param rr_b runtime_results of another driving regime.
   * @param ignore_border ignore the first and the last runtime result element,
   * default false
   * @return the intersection point given in meters.
   */
  static si::length intersection_point(runtime_results const& rr_a,
                                       runtime_results const& rr_b,
                                       bool ignore_border = true);

  /**
   * Calculates the intersection point of two driving regimes regarding the
   * velocity. One driving_regime has a non-constant speed between start and
   * end, the other driving regime has a constant speed (cruising).
   *
   * Check for a potential intersection.
   *
   * @param rr_a runtime_results of one non-constant driving regime
   * @param speed speed of a constant speed driving regime (cruising)
   *
   * @return the intersection point given in meters
   */
  static si::length intersection_point_with_constant(
      runtime_results const& rr_a, si::speed const& speed,
      bool const& search_first = true);
  /**
   *
   * Simulation of the current driving regime with given initial or final
   * velocity ( interpretation of the velocity depends on the reverse
   * parameter).
   *
   * @remark default implementation is switch between simulate_on and
   * simulate_reverse_on, but could be implemented with a concrete - specialized
   * - implementation
   *
   * @see concrete simulate_on and simulate_reverse_on implementation
   * @param reverse choose if simulation is reverse or not, default is false
   * @return a vector of runtime_results representing the simulation results
   */
  virtual std::vector<runtime_result> simulate(
      rs::train_physics const& tv, std::vector<interval> const& intervals,
      si::length start, si::length end, si::speed velocity, si::length offset,
      si::time time_offset, si::length step_size, bool reverse);
};

}  // namespace soro::runtime
