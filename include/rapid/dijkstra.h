#pragma once

#include <vector>

#include "rapid/network.h"

namespace rapid {

std::vector<edge*> dijkstra(network const& net, std::string_view from,
                            std::string_view to);

}  // namespace rapid