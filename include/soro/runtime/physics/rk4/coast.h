#pragma once

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/train_state.h"

namespace soro::runtime::rk4 {

// uses a runge kutta 4th order method to numerically solve the
// coasting differential equation
train_state coast(si::speed const initial_speed, si::length const dist,
                  si::slope const slope, rs::train_physics const& t);

}  // namespace soro::runtime::rk4
