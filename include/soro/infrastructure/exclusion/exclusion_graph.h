#pragma once

#include "soro/utls/container/offset_container.h"
#include "soro/utls/container/optional.h"

#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

struct exclusion_graph {
  soro::vector_map<interlocking_route::id, exclusion_set> nodes_;
};

}  // namespace soro::infra