#pragma once

#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct interlocking_subsystem {
  soro::vector<interlocking_route> interlocking_routes_;

  soro::vector<soro::vector<interlocking_route::id>> starting_at_;
  soro::vector<soro::vector<interlocking_route::id>> halting_at_;

  soro::vector<soro::vector<interlocking_route::id>> sr_to_participating_irs_;
  soro::vector<soro::vector<interlocking_route::id>> station_to_irs_;
  soro::vector<soro::vector<interlocking_route::id>> exclusions_;
};

}  // namespace soro::infra
