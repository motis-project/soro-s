#pragma once

#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

void connect_nodes(graph& g);

void connect_border(border const& from_border, border const& to_border,
                    graph& g);

}  // namespace soro::infra
