#pragma once

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/train_state.h"

namespace soro::runtime::rk4 {

// result is the delta for a single step with step size h
train_state rk4_step(si::speed const speed, si::time const h,
                     si::slope const slope, rs::train_physics const& tp);

}  // namespace soro::runtime::rk4