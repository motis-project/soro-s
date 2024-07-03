#pragma once

#include "soro/utls/container/static_vector.h"

#include "soro/timetable/train.h"

#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/signal_time.h"
#include "soro/runtime/common/train_safety.h"
#include "soro/runtime/common/train_state.h"
#include "soro/runtime/driver/command.h"

namespace soro::runtime {

struct drive_event {
  infra::node::ptr node_;
  si::time arrival_;
};

struct delta : train_state {
  static constexpr delta zero() { return {train_state::zero(), {}}; }
  std::vector<drive_event> events_;
};

using commands = utls::static_vector<command, 3>;

struct shortest_travel_time {
  delta drive(train_state const& initial, train_safety* train_safety,
              interval const& interval, tt::train const& train,
              tt::train::trip const& trip, signal_time const& signal_time);

private:
  si::speed get_max_speed(interval const& interval,
                          rs::train_physics const& tp) const;
  si::speed get_target_speed(interval const& interval,
                             rs::train_physics const& tp) const;

  void update(interval const& interval, tt::train const& train,
              tt::train::trip const& trip, train_state const& initial,
              signal_time const& signal_time);

  // currently approaching a main signal i.e. train is located between an
  // approach and a main signal
  bool approaching_{false};
};

}  // namespace soro::runtime
