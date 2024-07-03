#pragma once

#include "soro/ordering/graph.h"

namespace soro::ordering {

[[nodiscard]] bool has_exclusion_paths(graph::node const& start,
                                       graph const& og,
                                       infra::infrastructure const& infra);

bool has_exclusion_paths(graph const& og, infra::infrastructure const& infra);

}  // namespace soro::ordering
