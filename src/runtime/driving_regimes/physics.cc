#include "soro/runtime/driving_regimes/physics.h"

namespace soro::runtime {

std::tuple<si::time, si::speed> accelerate(si::acceleration const acc,
                                           si::speed const vel0,
                                           si::length const start,
                                           si::length const end) {
  utl::verify(
      vel0 >= si::speed{0.0F},
      "physics::accelerate: vel0 must be greater than or equal to zero.");
  utl::verify(start < end,
              "physics::accelerate: start must be before end of acceleration.");
  utl::verify(
      start >= si::length{0.0F},
      "physics::accelerate: start must be greater than or equal to zero .");
  utl::verify(end > si::length{0.0F},
              "physics::accelerate: end must be greater than zero.");

  // if acc is zero: no change in velocity
  if (acc == si::acceleration{0.0F}) {
    return std::make_tuple((end - start) / vel0, vel0);
  }

  // otherwise: velocity changes; calculate the time needed to move from start
  // to end by calculating dt from: 0 = dt^2 + 2 vel0/acc - 2(end-start)/acc
  double const discriminant =
      (pow((2 * vel0 / acc).val_, 2) - 4 * (-2 * (end - start) / acc).val_);

  // one can calculate a non complex dt if the discriminant is greater than or
  // equal to zero, see: https://en.wikipedia.org/wiki/Quadratic_equation
  if (discriminant < 0) {
    return std::make_tuple(si::time{-1.0F}, si::speed{0.0F});
  }

  auto const dt_candidates =
      general_driving::get_acceleration_duration(acc, vel0, end - start);

  // at this point it is known, that there is at least one real solution

  std::vector<si::time> valid_dt_candidates;

  // search for positive time periods
  if (std::get<0>(dt_candidates) >= si::time{0.0F}) {
    valid_dt_candidates.emplace_back(std::get<0>(dt_candidates));
  }
  if (std::get<1>(dt_candidates) >= si::time{0.0F}) {
    valid_dt_candidates.emplace_back(std::get<1>(dt_candidates));
  }

  // get the minimal time needed for the given interval (if there are two
  // positive real time candidates). Calculate the velocity at end
  auto delta_t =
      *std::min_element(valid_dt_candidates.begin(), valid_dt_candidates.end());
  auto vel1 = acc * delta_t + vel0;

  return std::make_tuple(delta_t, vel1);
}

std::tuple<si::time, si::speed> accelerate_reverse(si::acceleration acc,
                                                   si::speed vel1,
                                                   si::length start,
                                                   si::length end) {
  utl::verify(acc < si::acceleration{0.0F},
              "physics::accelerate_reverse: acc must be smaller than zero to"
              "use accelerate_reverse.");
  utl::verify(vel1 >= si::speed{0.0F},
              "physics::accelerate_reverse: vel1 must be greater than or equal "
              "to zero.");
  utl::verify(start >= si::length{0.0F},
              "physics::accelerate_reverse: start must be greater than or "
              "equal to zero.");
  utl::verify(end > si::length{0.0F},
              "physics::accelerate_reverse: end must be greater than zero.");
  utl::verify(
      start < end,
      "physics::accelerate_reverse: start must be before end of acceleration");

  // since acceleration is less than zero, the velocity will change; calculate
  // the time needed to move from start to end with the condition that the
  // velocity at the end of the interval is vel1 can be calculated by solving
  // 0 = dt^2 - 2 * vel1/acc + 2 * (end-start)/acc
  // to use get_acceleration_duration change the equation with the knowledge
  // that acc is negative to: 0 = dt^2 + 2 * vel1 /|acc| + 2 * (end-start)/|acc|

  // a non-complex time period can be calculated iff the discriminant is greater
  // than or equal to zero.
  double const discriminant = pow((2 * vel1 / (-1.0F * acc)).val_, 2.0F) -
                              4 * (-2 * (end - start) / (-1.0F * acc)).val_;
  utl::verify(discriminant >= 0,
              "physics::accelerate_reverse: no valid acceleration solution");

  auto const dt_candidates = general_driving::get_acceleration_duration(
      -1.0F * acc, vel1, end - start);

  // at this point it is known, that there is at least one solution

  std::vector<si::time> valid_dt_candidates;

  // search for positive time periods
  if (std::get<0>(dt_candidates) >= si::time{0.0F}) {
    valid_dt_candidates.emplace_back(std::get<0>(dt_candidates));
  }
  if (std::get<1>(dt_candidates) >= si::time{0.0F}) {
    valid_dt_candidates.emplace_back(std::get<1>(dt_candidates));
  }

  // get the minimal time needed for the given interval (if there are two
  // positive real time candidates). calculate vel0 with v1 = acc * dt + vel0
  auto delta_t =
      *std::min_element(valid_dt_candidates.begin(), valid_dt_candidates.end());
  auto vel0 = vel1 - acc * delta_t;

  return std::make_tuple(delta_t, vel0);
}

}  // namespace soro::runtime
