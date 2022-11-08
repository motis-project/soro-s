#pragma once

#include "soro/utls/serializable.h"

#include "soro/infrastructure/base_infrastructure.h"
#include "soro/infrastructure/infrastructure_options.h"

namespace soro::infra {

struct infrastructure : utls::serializable<base_infrastructure> {
  using utls::serializable<base_infrastructure>::serializable;

  explicit infrastructure(infrastructure_options const& options);
};

}  // namespace soro::infra
