#pragma once

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

soro::vector<element::ids> get_closed_exclusion_elements(
    infrastructure const& infra);

soro::vector<element::ids> get_open_exclusion_elements(
    soro::vector<element::ids> const& closed_exclusion_elements,
    infrastructure const& infra);

}  // namespace soro::infra
