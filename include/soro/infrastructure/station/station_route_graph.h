#pragma once

#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/station/station_route.h"

namespace soro::infra {

struct station_route_graph {
  soro::vector_map<station_route::id, soro::vector<station_route::ptr>>
      successors_;
  soro::vector_map<station_route::id, soro::vector<station_route::ptr>>
      predeccesors_;
};

station_route_graph get_station_route_graph(
    soro::vector_map<station_route::id, station_route::ptr> const&
        station_routes,
    infra::graph const& network);

}  // namespace soro::infra
