#pragma once

#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

struct critical_point {
  CISTA_COMPARABLE()

  enum type type(infrastructure const&) const { return type_; }

  element::id element_;
  // is either: switch, cross, route eotd
  enum type type_;
};

using critical_points = soro::vecvec<interlocking_route::id, critical_point>;

struct interlocking {
  soro::vector_map<interlocking_route::id, interlocking_route> routes_;

  // node n -> IRs starting at node n
  soro::vector_map<node::id, interlocking_route::ids> starting_at_;
  // node n -> IRs ending at node n
  soro::vector_map<node::id, interlocking_route::ids> ending_at_;

  // node n -> IRs halting at node n
  soro::vector_map<node::id, interlocking_route::ids> halting_at_;

  // station route to every participating interlocking route
  soro::vector_map<station_route::id, interlocking_route::ids> sr_to_irs_;
  soro::vector_map<station::id, interlocking_route::ids> station_to_irs_;

  critical_points critical_points_;
};

}  // namespace soro::infra
