#pragma once

#include "soro/si/units.h"

#include "soro/runtime/common/train_state.h"

namespace soro::runtime::rk4 {

train_state cruise(si::speed const speed, si::length const dist);

}  // namespace soro::runtime::rk4
