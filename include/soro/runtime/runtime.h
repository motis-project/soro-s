#pragma once

#include "soro/utls/unixtime.h"

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/runtime/runtime_physics.h"
#include "soro/timetable/timetable.h"

namespace soro::runtime {

struct timestamp {
  timestamp() = delete;
  timestamp(relative_time const arrival, relative_time const departure,
            infra::element::ptr const element)
      : arrival_{arrival}, departure_{departure}, element_{element} {}

  relative_time arrival_;
  relative_time departure_;
  //  utls::unixtime arrival_;
  //  utls::unixtime departure_;
  infra::element::ptr element_{nullptr};
};

struct timestamps {
  soro::vector<timestamp> times_;
  // every timestamp of a HALT in times_ will be indexed in halt_indices_.
  soro::vector<soro::size_t> halt_indices_;
};

/*
 * For every element type given in record_types the returned timestamps contain
 * a timestamp with arrival and departure value.
 */
timestamps runtime_calculation(tt::train const& train,
                               infra::infrastructure const& infra,
                               infra::type_set const& record_types);

}  // namespace soro::runtime
