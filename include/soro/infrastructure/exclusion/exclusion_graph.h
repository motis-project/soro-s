#pragma once

#include "soro/utls/container/offset_container.h"
#include "soro/utls/container/optional.h"

#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/graph/element.h"

namespace soro::infra {

struct exclusion_data {
  utls::optional<element_id> route_eotd_;
};

struct exclusion_graph {
  soro::vector<exclusion_set> nodes_;

  soro::vector<utls::offset_container<soro::vector<exclusion_data>>> data_;
};

}  // namespace soro::infra