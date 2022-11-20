#pragma once

#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct interlocking_subsystem {
  soro::vector<interlocking_route> interlocking_routes_;

  soro::map<node::id, soro::vector<interlocking_route::id>> starting_at_;
  //  soro::map<node::id, soro::vector<interlocking_route::id>> halting_at_;
  soro::vector<soro::vector<interlocking_route::id>> halting_at_;

  soro::vector<soro::vector<interlocking_route::id>> sr_to_participating_irs_;
  soro::vector<soro::vector<interlocking_route::id>> station_to_irs_;
  soro::vector<soro::vector<interlocking_route::ptr>> exclusions_;

  //  soro::vector<soro::unique_ptr<interlocking_route>>
  //  interlocking_route_store_;
};

}  // namespace soro::infra
