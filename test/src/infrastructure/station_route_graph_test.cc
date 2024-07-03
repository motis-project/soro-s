#include "doctest/doctest.h"

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra::test {

void check_station_route_graph(infrastructure const& infra) {
  for (auto const& sr : infra->station_routes_) {
    auto const& succs = infra->station_route_graph_.successors_[sr->id_];

    for (auto const& succ : succs) {
      CHECK_MESSAGE((succ->id_ != sr->id_),
                    "Station route can't be its own successor!");
    }
  }
}

void do_station_route_graph_tests(infrastructure const& infra) {
  check_station_route_graph(infra);
}

}  // namespace soro::infra::test