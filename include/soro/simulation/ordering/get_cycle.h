#pragma once

#include "soro/simulation/ordering/ordering_graph.h"

namespace soro::simulation {

using cycle = std::vector<ordering_node::id>;

std::optional<cycle> get_cycle(ordering_graph const& og);

std::vector<cycle> get_cycles(ordering_graph const& og);

}  // namespace soro::simulation