#pragma once

#include "soro/infrastructure/base_infrastructure.h"
#include "soro/infrastructure/infrastructure_options.h"

namespace soro::infra {

base_infrastructure parse_iss(infrastructure_options const& options);

}  // namespace soro::infra