#pragma once

#include "soro/base/time.h"

#include "soro/infrastructure/graph/element.h"

namespace soro::runtime {

struct signal_time {
  infra::element::id approach_{infra::element::id::invalid()};
  infra::element::id main_{infra::element::id::invalid()};
  absolute_time time_{ZERO<absolute_time>};
};

}  // namespace soro::runtime
