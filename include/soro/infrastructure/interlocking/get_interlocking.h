#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking.h"

namespace soro::infra {

interlocking get_interlocking(infrastructure_t const& infra);

}  // namespace soro::infra
