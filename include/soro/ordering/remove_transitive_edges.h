#pragma once

#include "soro/ordering/graph.h"

namespace soro::ordering {

void remove_transitive_dependency_edges(
    soro::vector<soro::vector<graph::node::id>>& outgoing, graph const& og);

}  // namespace soro::ordering
