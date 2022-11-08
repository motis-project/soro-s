#pragma once

#include "soro/base/soro_types.h"

#include "soro/infrastructure/interlocking/get_interlocking_subsystem.h"

namespace soro::infra {

soro::vector<soro::vector<ir_ptr>> get_ssr_conflicts(
    base_infrastructure const& iss, interlocking_subsystem const&);

soro::vector<element_ptr> get_exclusion_elements(
    interlocking_route const& ssr, base_infrastructure const& iss);

}  // namespace soro::infra