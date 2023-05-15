#pragma once

#include "soro/simulation/ordering/ordering_graph.h"

namespace soro::simulation {

[[nodiscard]] bool has_exclusion_paths(ordering_graph const& og,
                                       infra::infrastructure const& infra);

}  // namespace soro::simulation
