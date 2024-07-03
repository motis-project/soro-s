#pragma once

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/runtime_result.h"

namespace soro::runtime::euler {

runtime_results accelerate(rs::train_physics const& tp, si::speed initial_speed,
                           si::speed target_speed, si::length max_distance,
                           si::slope slope);

runtime_results brake(si::speed const initial_speed,
                      si::speed const target_speed,
                      si::accel const deacceleration);

runtime_results cruise(si::speed const current_speed,
                       si::length const distance);

}  // namespace soro::runtime::euler