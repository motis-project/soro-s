#pragma once

#include "soro/infrastructure/exclusion/exclusion_graph.h"
#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct exclusion {
  // all maximal cliques in the exclusion graph
  soro::vector<interlocking_route::ids> exclusion_sets_;

  // maps IRs to their cliques in the exclusion graph
  soro::vector_map<interlocking_route::id, exclusion_set::ids>
      irs_to_exclusion_sets_;
};

}  // namespace soro::infra
