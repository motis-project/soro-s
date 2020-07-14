#pragma once

#include <vector>

#include "soro/network.h"

namespace soro {

std::vector<edge*> dijkstra(network const&, node* from, node* to);
std::vector<edge*> dijkstra(network const& net, std::string_view from,
                            std::string_view to);

}  // namespace soro