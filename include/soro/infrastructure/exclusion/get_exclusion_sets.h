#pragma once

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

soro::vector<exclusion_set> get_exclusion_sets(
    infrastructure const& infra,
    soro::vector<interlocking_route::ids> const& closed_element_used_by,
    soro::vector<element::ids> const& open_exclusion_elements);

}  // namespace soro::infra
