#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

namespace soro::runtime {

struct event {
  event() = delete;
  event(infra::node_ptr const node, si::length const dist)
      : node_(node), distance_(dist) {}

  auto type() const { return node_->element_->type(); }

  infra::node_ptr node_{nullptr};
  si::length distance_{si::ZERO<si::length>};
};

struct interval {
  interval() = delete;
  interval(si::length const dist, si::speed const limit)
      : distance_(dist), speed_limit_(limit) {}
  interval(si::length const dist, si::speed const limit, bool const halt,
           infra::type const element)
      : distance_(dist),
        speed_limit_(limit),
        halt_(halt),
        elements_({element}) {}

  si::length distance_{si::ZERO<si::length>};
  si::speed speed_limit_{si::ZERO<si::speed>};
  bool halt_{false};  // true if elements_ contains a type::HALT

  std::vector<event> events_;
  std::vector<infra::type> elements_;
};

using interval_list = soro::vector<interval>;

interval_list get_interval_list(tt::train const& tr,
                                infra::type_set const& event_types,
                                infra::type_set const& border_types,
                                infra::infrastructure const& infra);

}  // namespace soro::runtime
