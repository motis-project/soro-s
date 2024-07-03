#pragma once

#include "soro/base/soro_types.h"

#include "soro/si/units.h"

#include "soro/runtime/common/interval.h"
#include "soro/runtime/strategy/shortest_travel_time.h"

namespace soro::runtime {

struct driver {

  delta drive(train_state const state, train_safety* train_safety,
              interval const& interval, tt::train const& train,
              tt::train::trip const& trip, signal_time const& signal_time) {

    return stt_.drive(state, train_safety, interval, train, trip, signal_time);
  }

  shortest_travel_time stt_;
};

}  // namespace soro::runtime
