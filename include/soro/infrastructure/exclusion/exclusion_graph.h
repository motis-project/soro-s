#pragma once

#include "soro/utls/container/offset_container.h"
#include "soro/utls/container/optional.h"

#include "soro/infrastructure/exclusion/exclusion_set.h"

namespace soro::infra {

struct exclusion_data {
  utls::optional<uint16_t> node_offset_;
};

struct exclusion_graph {
  soro::vector<exclusion_set> nodes_;

  soro::vector<utls::offset_container<soro::vector<exclusion_data>>>
      exclusion_data_;
};

}  // namespace soro::infra