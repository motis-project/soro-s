#pragma once

#include "soro/simulation/ordering/ordering_graph.h"

namespace soro::simulation {

void remove_transitive_edges(ordering_graph& og);

}  // namespace soro::simulation
