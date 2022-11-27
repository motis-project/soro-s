#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_subsystem.h"

namespace soro::infra {

interlocking_subsystem get_interlocking_subsystem(
    infrastructure_t const& infra, bool const determine_conflicts);

}  // namespace soro::infra
