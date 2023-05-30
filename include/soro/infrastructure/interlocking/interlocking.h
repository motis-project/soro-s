#pragma once

#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct critical_point {
  auto operator<=>(critical_point const&) const = default;

  enum type type(infrastructure const&) const { return type_; }

  element_id element_;
  // is either: switch, cross, route eotd
  enum type type_;
};

using critical_points = soro::vecvec<interlocking_route::id, critical_point>;

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

  critical_points critical_points_;
};

}  // namespace soro::infra
