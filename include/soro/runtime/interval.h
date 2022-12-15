#pragma once

#include "utl/concat.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

namespace soro::runtime {

struct event {
  event() = delete;
  event(infra::node::ptr const node, si::length const dist)
      : node_(node), distance_(dist) {}

  auto type() const { return node_->element_->type(); }

  infra::node::ptr node_{nullptr};
  si::length distance_{si::ZERO<si::length>};
};

struct interval {
  interval(si::length const dist, si::speed const left_limit,
           si::speed const right_limit, tt::sequence_point::optional_ptr sp,
           std::vector<event> events)
      : distance_(dist),
        limit_left_{left_limit},
        limit_right_{right_limit},
        sequence_point_{sp},
        events_{std::move(events)} {}

  void append(interval const& o) {
    limit_right_ = o.limit_right_;
    utl::concat(events_, o.events_);

    utls::sassert(
        !(sequence_point_.has_value() && o.sequence_point_.has_value()),
        "Appending two intervals where both have a sequence point");

    if (o.sequence_point_.has_value()) {
      sequence_point_ = o.sequence_point_;
    }
  }

  bool is_halt() const { return sequence_point_.has_value(); }

  duration2 min_stop_time() const {
    utls::sassert(is_halt(), "Asking for min stop time on non-halt interval");
    return (*sequence_point_)->min_stop_time_;
  }

  relative_time departure() const {
    utls::sassert(is_halt(), "Asking for departure time on non-halt interval");
    return (*sequence_point_)->departure_;
  }

  si::speed target_speed() const {
    return is_halt() ? si::ZERO<si::speed> : limit_right_;
  }

  si::speed target_speed(rs::train_physics const& tp) const {
    return std::min(target_speed(), tp.max_speed());
  }

  si::length distance_{si::INVALID<si::length>};
  si::speed limit_left_{si::INVALID<si::speed>};
  si::speed limit_right_{si::INVALID<si::speed>};

  tt::sequence_point::optional_ptr sequence_point_{};

  std::vector<event> events_;
};

using interval_list = soro::vector<interval>;

interval_list get_interval_list(tt::train const& train,
                                infra::type_set const& event_types,
                                infra::infrastructure const& infra);

}  // namespace soro::runtime
