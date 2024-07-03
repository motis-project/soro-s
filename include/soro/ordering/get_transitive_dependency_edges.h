#pragma once

#include "soro/ordering/graph.h"

namespace soro::ordering {

soro::vector<graph::edge> get_transitive_dependency_edges(
    graph::node::id const start,
    soro::vector<soro::vector<graph::node::id>> const& outgoing,
    graph const& og);

}  // namespace soro::ordering
