#pragma once

#include "soro/infrastructure/exclusion/exclusion_elements.h"
#include "soro/infrastructure/exclusion/exclusion_graph.h"
#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

exclusion_graph get_exclusion_graph(infrastructure const& infra);

exclusion_graph get_exclusion_graph(
    soro::vector<element::ids> const& closed_exclusion_elements,
    soro::vector<interlocking_route::ids> const& closed_element_used_by,
    infrastructure const& infra);

}  // namespace soro::infra
