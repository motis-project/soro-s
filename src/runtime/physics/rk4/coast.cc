#include "soro/runtime/physics/rk4/coast.h"

#include "soro/utls/sassert.h"

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/runtime_result.h"

namespace soro::runtime::rk4 {

using namespace soro::rs;

train_state coast(si::speed const, si::length const, si::slope const,
                  train_physics const&) {
  utls::ensure(false, "not implemented");

  train_state result;
  //  result.speed_ = initial_speed;
  //  result.dist_ = si::length::zero();
  //  result.time_ = si::time::zero();
  //
  //  while (result.speed_ > si::speed::zero() && result.dist_ < dist) {
  //    result += rk4_step(result.speed_, delta_t, slope, tp);
  //  }
  //
  //  result.dist_ = std::min(dist, result.dist_);

  return result;
}

}  // namespace soro::runtime::rk4