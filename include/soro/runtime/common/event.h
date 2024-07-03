#pragma once

#include "soro/base/time.h"

#include "soro/infrastructure/graph/element.h"

namespace soro::runtime {

struct event : times {
  auto operator<=>(event const&) const = default;

  infra::element::ptr element_{nullptr};
};

using EventCB = std::function<void(event const&)>;

}  // namespace soro::runtime