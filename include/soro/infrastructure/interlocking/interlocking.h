#pragma once

#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct interlocking {
  soro::vector<interlocking_route> routes_;

  // node n -> IRs starting at node n
  soro::vector<interlocking_route::ids> starting_at_;
  // node n -> IRs ending at node n
  soro::vector<interlocking_route::ids> ending_at_;

  // node n -> IRs halting at node n
  soro::vector<interlocking_route::ids> halting_at_;

  soro::vector<interlocking_route::ids> sr_to_participating_irs_;
  soro::vector<interlocking_route::ids> station_to_irs_;
};

}  // namespace soro::infra
