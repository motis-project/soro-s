#pragma once

#include "soro/infrastructure/exclusion/exclusion_graph.h"
#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct exclusion_elements {
  // contains the exclusion elements for each interlocking route
  // [start, ..., end], i.e. a closed interval
  soro::vector<element::ids> closed_;
  // contains the exclusion elements for each interlocking route, w/o start end
  // (start, ..., end), i.e. an open interval
  soro::vector<element::ids> open_;
};

struct exclusion {
  exclusion_elements exclusion_elements_;

  exclusion_graph exclusion_graph_;

  // all maximal cliques in the exclusion graph
  soro::vector<interlocking_route::ids> exclusion_sets_;

  // maps IRs to their cliques in the exclusion graph
  soro::vector<exclusion_set::ids> irs_to_exclusion_sets_;
};

}  // namespace soro::infra
