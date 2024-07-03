#include "soro/runtime/physics/rk4/cruise.h"

#include "soro/utls/sassert.h"

#include "soro/si/units.h"

#include "soro/runtime/common/runtime_result.h"

namespace soro::runtime::rk4 {

train_state cruise(si::speed const speed, si::length const dist) {
  utls::expect(speed.is_valid(), "speed invalid");
  utls::expect(dist.is_valid(), "distance invalid");

  utls::expect(speed > si::speed::zero(), "speed must be greater than zero");
  utls::expect(dist > si::length::zero(), "distance must be greater than zero");

  utls::expect(!speed.is_infinity(), "no infinity");
  utls::expect(!dist.is_infinity(), "no infinity");

  train_state result;

  result.time_ = dist / speed;
  result.speed_ = speed;
  result.dist_ = dist;

  utls::ensure(!result.time_.is_infinity(), "no infinity");

  return result;
}

}  // namespace soro::runtime::rk4
