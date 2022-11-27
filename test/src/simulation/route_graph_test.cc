#include "doctest/doctest.h"

#include <filesystem>
#include <vector>

#include "utl/enumerate.h"

#include "soro/infrastructure/infrastructure_t.h"
#include "soro/simulation/sim_graph.h"
#include "soro/timetable/timetable.h"
#include "soro/utls/parallel_for.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::utls;
using namespace soro::tt;
using namespace soro::simulation;
using namespace soro::infra;

/*
 * TODO(julian) port these tests to sim_graph
 void compare_paths(route_graph const& rg) {
   for (auto const& [train_name, node_ids] : rg.train_name_to_nodes_) {
     std::vector<node_ptr> signal_station_route_path;
     std::vector<node_ptr> station_route_path;

     size_t main_signal_count = 0;
     size_t eotd_count = 0;

     size_t main_route_count = 0;
     size_t partial_route_count = 0;

     for (auto const r_id : node_ids) {
       auto const& route_node = rg.get(r_id);

       for (auto const& i_node : route_node.nodes_) {
         signal_station_route_path.push_back(i_node);
       }

       if (route_node.is_partial()) {
         ++partial_route_count;
       } else {
         ++main_route_count;
       }
     }

     auto const& train_run = *rg.get(node_ids.front()).train_;

     bool skip = true;
     for (auto const [e_idx, entry] : utl::enumerate(train_run.entries_)) {
       for (auto const [sr_idx, sr] : utl::enumerate(entry.station_routes_)) {
         for (auto const [n_idx, i_node] : utl::enumerate(sr->nodes_)) {
           auto const halt = sr->get_halt_idx(train_run.freight_);

           bool const first_route = e_idx == 0 && sr_idx == 0;

           if (first_route && n_idx == halt) {
             skip = false;
           }

           if (skip) {
             continue;
           }

           station_route_path.push_back(i_node);

           if (i_node->is(type::MAIN_SIGNAL)) {
             ++main_signal_count;
           }

           if (i_node->is(type::EOTD)) {
             ++eotd_count;
           }

           bool const last_route = e_idx == train_run.entries_.size() - 1 &&
                                   sr_idx == entry.station_routes_.size() - 1;
           if (last_route && n_idx == halt) {
             break;
           }
         }
       }
     }

     CHECK_MESSAGE(signal_station_route_path.size() ==
     station_route_path.size(),
                   "Differing path lengths from base_infrastructure station
                   route path and " "signal station route path");

     for (size_t idx = 0; idx < signal_station_route_path.size(); ++idx) {
       CHECK_MESSAGE(signal_station_route_path[idx] ==
       station_route_path[idx],
                     "Differing elements in base_infrastructure station route
                     path and signal " "station route path");
     }

     CHECK_MESSAGE(main_signal_count + 1 == main_route_count,
                   "There must be one more main route than there are main "
                   "signals on the path");

     CHECK_MESSAGE(eotd_count - main_route_count == partial_route_count,
                   "Every main route gets one ETOD, all others should belong
                   to " "a partial route");
   }
 }

 void check_dependencies(route_graph const& rg) {
   for (auto const& [train_name, route_node_ids] : rg.train_name_to_nodes_) {

     for (auto idx = 0UL; idx < route_node_ids.size() - 1; ++idx) {
       auto route_node = rg.get(route_node_ids[idx]);
       auto next_route_node = rg.get(route_node_ids[idx + 1]);

       CHECK_MESSAGE(route_node.same_train_succ() == route_node_ids[idx + 1],
                     "Same train successor must be the next route node in
                     line");
       CHECK_MESSAGE(
           next_route_node.same_train_dep() == route_node_ids[idx],
           "Same train dependency must be the last route node in line");
     }
   }
 }

 void edge_check(route_graph const& rg) {
   auto const& check_incoming = [](auto const& route_node) {
     // only main routes can have incoming edges
     // partial routes must not have incoming edges

     if (!route_node.is_partial()) {
       return;
     }

     if (route_node.is_init()) {
       CHECK_MESSAGE(route_node.in_.empty(),
                     "Init nodes are not allowed to have incoming edges");
     }

     if (route_node.is_partial()) {
       CHECK_MESSAGE(route_node.in_.size() == 1,
                     "Partial nodes must have exactly one incoming edge");
     }
   };

  auto const& check_outgoing = [&rg](route_node const& route_node) {
    auto const& conflicts = rg.conflicts_[route_node.id_];

    for (auto const& conflict_id : conflicts) {
      // there must be a path between a route node r1 and a route node r2 if
      // they both are in conflict. the path is either r1 -> r2 or r2 -> r1
      bool const r1_to_r2 = path_exists(rg, route_node.id_, conflict_id);
      bool const r2_to_r1 = path_exists(rg, conflict_id, route_node.id_);
      auto const path_exists = r1_to_r2 || r2_to_r1;

      CHECK_MESSAGE(path_exists,
                    "Found no path between conflicting route nodes!");

      CHECK_MESSAGE(r1_to_r2 != r2_to_r1, "Found cycle in graph");
    }
  };

  auto const& check_edges = [&](auto const& route_node) {
    check_incoming(route_node);
    check_outgoing(route_node);
  };

  utls::parallel_for(rg.nodes_, check_edges);
}

 void check_route_graph(route_graph const& rg) {
   compare_paths(rg);
   check_dependencies(rg);
   edge_check(rg);
}
*/

TEST_CASE("build_soro-s_graph_overtake") {  // NOLINT
  infrastructure const infra(soro::test::SMALL_OPTS);
  timetable const tt(soro::test::OVERTAKE_OPTS, infra);
}

TEST_CASE("build_soro-s_graph_follow") {  // NOLINT
  infrastructure const infra(soro::test::SMALL_OPTS);
  timetable const tt(soro::test::FOLLOW_OPTS, infra);

  sim_graph const sg(infra, tt);
}