#pragma once

#include "soro/infrastructure/graph/type.h"

#include "soro/runtime/common/event.h"

namespace soro::runtime {

struct timestamps {
  soro::vector<event> times_;
  std::map<infra::type, soro::vector<soro::size_t>> type_indices_;
};

}  // namespace soro::runtime
