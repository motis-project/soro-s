#pragma once

#include "soro/si/units.h"

#include "soro/runtime/common/train_state.h"

namespace soro::runtime::rk4 {

train_state brake(si::speed const initial_speed, si::speed const target_speed,
                  si::accel const deaccel);

train_state brake_over_distance(si::speed const initial_speed,
                                si::accel const deaccel,
                                si::length const distance);

}  // namespace soro::runtime::rk4
