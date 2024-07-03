#pragma once

#include "soro/infrastructure/infrastructure_t.h"

namespace soro::infra {

soro::vector_map<node::id, coordinates> get_node_coordinates(
    soro::vector_map<element::id, coordinates> const& element_coordinates,
    infrastructure_t const& infra);

}  // namespace soro::infra
