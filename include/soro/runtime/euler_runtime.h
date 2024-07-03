#pragma once

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/timetable.h"

#include "soro/runtime/common/event.h"
#include "soro/runtime/common/terminate.h"
#include "soro/runtime/common/timestamps.h"
#include "soro/runtime/common/use_surcharge.h"

namespace soro::runtime::euler {

/*
 * For every element type given in record_types the returned timestamps contain
 * a timestamp with arrival and departure value.
 */
timestamps runtime_calculation(tt::train const& t,
                               infra::infrastructure const& infra,
                               infra::type_set const& record_types,
                               use_surcharge const use_surcharge);

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

}  // namespace soro::runtime::euler
