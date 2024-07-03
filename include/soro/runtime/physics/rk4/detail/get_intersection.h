#pragma once

#include "soro/si/units.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/runtime/common/train_state.h"
#include "soro/runtime/physics/rk4/detail/get_speed_limit.h"

namespace soro::runtime::rk4::detail {

train_state get_intersection(train_state const second_to_last,
                             train_state const last, si::accel const deaccel,
                             si::slope const slope,
                             get_speed_limit const& get_speed_limit,
                             rs::train_physics const& tp);

train_state get_intersection_at_max_dist(si::length const length,
                                         train_state const second_to_last,
                                         train_state const last,
                                         si::slope const slope,
                                         rs::train_physics const& tp);

}  // namespace soro::runtime::rk4::detail
