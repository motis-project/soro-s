#include "test/ordering/has_exclusion_paths.h"

#include <atomic>
#include <vector>

#include "utl/logging.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/graph/traversal.h"
#include "soro/utls/print_progress.h"
#include "soro/utls/std_wrapper/all_of.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "soro/ordering/graph.h"

namespace soro::ordering {

std::vector<bool> get_reachable_nodes(graph const& og,
                                      graph::node::id const& start) {
  std::vector<bool> reachable(og.nodes_.size(), false);
  reachable[start.v_] = true;

  auto const work_on_node = [&reachable](auto&& to, auto&&) {
    reachable[to.v_] = true;
    return false;
  };

  auto const get_neighbours = [&og](auto&& node_id) {
    return og.nodes_[node_id].out(og);
  };

  utls::bfs(start, get_neighbours, work_on_node);

  return reachable;
}

bool path_exists(graph const& og, graph::node::id const source,
                 graph::node::id const target) {

  bool reachable = false;

  auto const work_on_node = [&reachable, target](auto&& to, auto&&) {
    if (to == target) {
      reachable = true;
      return true;
    }

    return false;
  };

  auto const get_neighbours = [&og](auto&& node_id) {
    return og.nodes_[node_id].out(og);
  };

  utls::bfs(source, get_neighbours, work_on_node);

  return reachable;
}

std::vector<bool> get_exclusions(infra::interlocking_route::id const ir_id,
                                 infra::infrastructure const& infra) {
  std::vector<bool> exclusions(infra->interlocking_.routes_.size(), false);

  for (auto const set_id : infra->exclusion_.irs_to_exclusion_sets_[ir_id]) {
    for (auto const& excluded_ir : infra->exclusion_.exclusion_sets_[set_id]) {
      exclusions[as_val(excluded_ir)] = true;
    }
  }

  return exclusions;
}

bool has_exclusion_paths(graph::node const& start, graph const& og,
                         infra::infrastructure const& infra) {
  auto const reachable = get_reachable_nodes(og, start.get_id(og));

  auto const ir1 = infra->interlocking_.routes_[start.ir_id_];
  auto const exclusions = get_exclusions(ir1.id_, infra);

  auto const has_path_impl = [&](auto&& target) {
    auto const start_id = start.get_id(og);
    auto const target_id = target.get_id(og);

    if (start_id == target_id) {
      return true;
    }

    auto const ir2 = infra->interlocking_.routes_[target.ir_id_];

    // ir1 not in exclusion with ir2
    // => no path must exist
    if (!exclusions[as_val(ir2.id_)]) return true;

    // ir1 in exclusion with ir2
    // => there must be a path from start to target
    bool const start_target_path = reachable[target_id.v_];
    bool const target_start_path =
        start_target_path || path_exists(og, target_id, start_id);  // NOLINT
    bool const path_exists = start_target_path || target_start_path;

    if (!path_exists) {
      uLOG(utl::warn) << "no exclusion path from node " << start.print(og)
                      << " to " << target.print(og);
    }

    return path_exists;
  };

  return utls::all_of(og.nodes_, has_path_impl);
}

bool has_exclusion_paths(graph const& og, infra::infrastructure const& infra) {
  utl::scoped_timer const timer("checking for exclusion paths");

  std::atomic<bool> result = true;

  utl::parallel_for(og.nodes_, [&](auto&& node) {
    if (!result) return;

    utls::print_progress("checking exclusion paths", og.nodes_);
    auto const has_paths = has_exclusion_paths(node, og, infra);

    if (!has_paths) result = false;
  });

  return result;
}

}  // namespace soro::ordering
