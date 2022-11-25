#include "soro/simulation/ordering_graph.h"

#include "utl/enumerate.h"

#include "soro/utls/container/priority_queue.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

#include "soro/runtime/runtime.h"

namespace soro::simulation {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;

// route_ordering get_route_ordering(infra::infrastructure const& infra,
//                                   tt::timetable const& tt) {
//   route_ordering ordering(infra->ssr_manager_.signal_station_routes_.size());
//
//   for (auto const& tr : tt->train_runs_) {
//     auto const stamps = runtime_calculation(*tr, *infra,
//     {type::MAIN_SIGNAL});
//
//     utl::verify(tr->ssr_run_.path_.size() == stamps.times_.size() + 1,
//                 "Differing amounts of signal station routes in train path and
//                 " "main signals in running time calculation timestamps");
//
//     for (auto const [idx, ssr] : utl::enumerate(tr->ssr_run_.path_)) {
//       auto const from =
//           idx == 0 ? tr->first_departure() : stamps.times_[idx -
//           1].departure_;
//       auto const to = idx == tr->ssr_run_.path_.size() - 1
//                           ? tr->last_arrival()
//                           : stamps.times_[idx].arrival_;
//
//       // the train tr will actually use the signal station route ssr
//       //  -> insert a usage for it
//       //      ordering[ssr->id_].push_back(
//       //          {.from_ = from, .to_ = to, .train_id_ = tr->id_});
//
//       // the train tr will block signal station routes in conflict with ssr
//       //  -> insert a usage for all conflicting routes
//       for (auto const& conflict : infra->ssr_manager_.conflicts_[ssr->id_]) {
//         ordering[conflict->id_].push_back(
//             {.from_ = from, .to_ = to, .train_id_ = tr->id_});
//       }
//     }
//   }
//
//   for (auto& usage_order : ordering) {
//     utls::sort(usage_order, [](auto&& usage1, auto&& usage2) {
//       return usage1.from_ < usage2.from_;
//     });
//   }
//
//   return ordering;
// }
//
// usage_index get_usage_index(route_ordering const& ordering,
//                             tt::dispo_train_run_id const train,
//                             signal_station_route::id const ssr) {
//   for (auto const [idx, order] : utl::enumerate(ordering[ssr])) {
//     if (order.train_id_ == train) {
//       return idx;
//     }
//   }
//
//   throw utl::fail(
//       "Could not find train with ID {} using signal station route with ID
//       {}.", train, ssr);
// }

struct route_usage {
  utls::unixtime from_{utls::INVALID_TIME};
  utls::unixtime to_{utls::INVALID_TIME};
  ordering_node::id node_id_{ordering_node::INVALID};
};

using usage_idx = uint32_t;
constexpr auto const INVALID_USAGE_IDX = std::numeric_limits<usage_idx>::max();

ordering_graph::ordering_graph(infra::infrastructure const& infra,
                               tt::timetable const& tt) {

  std::vector<std::vector<route_usage>> route_orderings(
      infra->interlocking_.interlocking_routes_.size());

  for (auto const& train : tt) {
    auto const stamps = runtime_calculation(*train, infra, {type::MAIN_SIGNAL});

    utl::verify(train->path_.size() == stamps.times_.size() + 1,
                "Differing amounts of signal station routes in train path and "
                "main signals in running time calculation timestamps");

    for (auto const [idx, ir_id] : utl::enumerate(train->path_)) {
      auto const id = static_cast<ordering_node::id>(nodes_.size());

      nodes_.emplace_back(id, ir_id, train->id_);

      auto const from = idx == 0 ? train->first_departure()
                                 : stamps.times_[idx - 1].departure_;
      auto const to = idx == train->path_.size() - 1
                          ? train->last_arrival()
                          : stamps.times_[idx].arrival_;

      route_orderings[ir_id].push_back(
          {.from_ = from, .to_ = to, .node_id_ = id});
    }
  }

  for (auto& usage_order : route_orderings) {
    utls::sort(usage_order, [](auto&& usage1, auto&& usage2) {
      return usage1.from_ < usage2.from_;
    });
  }

  // holds an index for every std::vector<route_usage> in route_orderings
  // the index points to the smallest not yet processed route_usage object
  std::vector<size_t> current_route_usage_index(route_orderings.size(),
                                                INVALID_USAGE_IDX);

  auto const entry_comparison = [&](auto const e1, auto const e2) {
    return route_orderings[e1][current_route_usage_index[e1]].from_ >
           route_orderings[e2][current_route_usage_index[e2]].from_;
  };

  // the priority queue contains every interlocking route that is used
  // by at least one train contained in the timetable.
  // the elements of the queue are sorted so that the top element is the
  // usage of an interlocking route with the smallest from_ time value of its
  // usage w.r.t to the current route usage index
  utls::priority_queue<ir_id, decltype(entry_comparison)> todo_queue(
      entry_comparison);

  for (auto const [route_id, usages] : utl::enumerate(route_orderings)) {
    if (usages.empty()) {
      continue;
    }

    current_route_usage_index[route_id] = 0;
    todo_queue.emplace(static_cast<ir_id>(route_id));
  }

  // the algorithm is similar to a merge operation
  // take the currently smallest route usage and add edges to all other
  // nodes with interlocking routes that are excluded
  // this is correct since we have taken the smallest (earliest) usage,
  // so all other nodes (i.e. usages) follow the currently processed usage
  // then increment the usage index of the interlocking route and re-add it
  // to the queue
  while (!todo_queue.empty()) {
    auto from_ssr = todo_queue.top();
    todo_queue.pop();

    auto& from_idx = current_route_usage_index[from_ssr];
    auto const from_usage = route_orderings[from_ssr][from_idx];
    auto const from_node = from_usage.node_id_;

    for (auto const& to_ir : infra->interlocking_.exclusions_[from_ssr]) {
      auto to_idx = current_route_usage_index[to_ir];

      if (to_idx == INVALID_USAGE_IDX) {
        continue;
      }

      // every interlocking route is in conflict with itself
      // since we are currently processing the usage of an interlocking
      // route we have to manually increment the usage index to get the next
      // usage of a different train of the same interlocking route
      if (to_ir == from_ssr) {
        if (to_idx < route_orderings[to_ir].size() - 1) {
          ++to_idx;
        } else {
          // if its the last usage of the interlocking route we can simply
          // continue, as we there will not be an edge
          continue;
        }
      }

      auto const& to_usage = route_orderings[to_ir][to_idx];
      auto const to_node = to_usage.node_id_;

      nodes_[from_node].out_.emplace_back(to_node);
      nodes_[to_node].in_.emplace_back(from_node);
    }

    if (from_idx < route_orderings[from_ssr].size() - 1) {
      ++from_idx;
      todo_queue.emplace(from_ssr);
    }
  }
}

}  // namespace soro::simulation