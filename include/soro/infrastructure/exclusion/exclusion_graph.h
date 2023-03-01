#pragma once

#include "soro/infrastructure/exclusion/exclusion_set.h"

namespace soro::infra {

struct exclusion_graph {
  soro::vector<exclusion_set> nodes_;
};

}  // namespace soro::infra