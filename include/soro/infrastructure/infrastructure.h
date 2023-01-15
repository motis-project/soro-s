#pragma once

#include "soro/utls/serializable.h"

#include "soro/infrastructure/infrastructure_options.h"
#include "soro/infrastructure/infrastructure_t.h"

namespace soro::infra {

struct infrastructure : utls::serializable<infrastructure_t> {
  using utls::serializable<infrastructure_t>::serializable;

  // TODO(julian) This wrapper is only a temporary solution
  explicit infrastructure(infrastructure_t const* wrapped_infra);

  explicit infrastructure(infrastructure_options const& options);
};

}  // namespace soro::infra
