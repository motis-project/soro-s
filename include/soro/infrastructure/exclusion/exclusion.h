#pragma once

#include "soro/infrastructure/exclusion/exclusion_graph.h"
#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct exclusion {
  // contains the exclusion elements for each interlocking route
  // [start, ..., end], i.e. a closed interval
  soro::vector<element::ids> closed_exclusion_elements_;
  // contains the exclusion elements for each interlocking route, w/o start end
  // (start, ..., end), i.e. an open interval
  soro::vector<element::ids> open_exclusion_elements_;

  exclusion_graph exclusion_graph_;

  // all maximal cliques in the exclusion graph
  soro::vector<exclusion_set> exclusion_sets_;

  // maps IRs to their cliques in the exclusion graph
  soro::vector<exclusion_set::ids> irs_to_exclusion_sets_;
};

}  // namespace soro::infra
