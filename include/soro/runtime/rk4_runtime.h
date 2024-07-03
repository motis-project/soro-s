#pragma once

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/train.h"

#include "soro/runtime/common/event.h"
#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/runtime_result.h"
#include "soro/runtime/common/terminate.h"
#include "soro/runtime/common/timestamps.h"
#include "soro/runtime/common/use_surcharge.h"

#include "soro/runtime/driver/driver.h"

namespace soro::runtime::rk4 {

struct runtime_state {
  driver driver_;
  train_state train_state_;
  train_safety train_safety_;
};

void calculate_interval(interval const& i, tt::train const& train,
                        tt::train::trip const& trip, runtime_state& current,
                        use_surcharge const use_surcharge,
                        signal_time const& signal_time,
                        EventCB const& event_cb);

relative_time runtime_calculation(tt::train const& t,
                                  infra::infrastructure const& infra,
                                  infra::type_set const& record_types,
                                  use_surcharge const use_surcharge,
                                  EventCB const& event_cb,
                                  TerminateCb const& terminate_cb);

relative_time runtime_calculation(tt::train const& t,
                                  infra::infrastructure const& infra,
                                  infra::type_set const& record_types,
                                  use_surcharge const use_surcharge,
                                  EventCB const& event_cb);

timestamps runtime_calculation(tt::train const& t,
                               infra::infrastructure const& infra,
                               infra::type_set const& record_types,
                               use_surcharge const use_surcharge);

}  // namespace soro::runtime::rk4