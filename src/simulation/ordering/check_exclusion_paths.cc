#include "soro/simulation/ordering/check_exclusion_paths.h"

#include "soro/utls/graph/traversal.h"
#include "soro/utls/std_wrapper/all_of.h"

using namespace soro::infra;

namespace soro::simulation {

std::vector<bool> get_reachable_nodes(ordering_graph const& og,
                                      ordering_node::id const& start) {
  std::vector<bool> reachable(og.nodes_.size(), false);
  reachable[start] = true;

  auto const work_on_node = [&reachable](auto&& to, auto&&) {
    reachable[to] = true;
    return false;
  };

  auto const get_neighbours = [&og](auto&& node_id) -> auto const& {
    return og.nodes_[node_id].out_;
  };

  utls::bfs(start, get_neighbours, work_on_node);

  return reachable;
}

bool path_exists(ordering_graph const& og, ordering_node::id const start,
                 ordering_node::id const target) {

  bool reachable = false;

  auto const work_on_node = [&reachable, target](auto&& to, auto&&) {
    if (to == target) {
      reachable = true;
      return true;
    }

    return false;
  };

  auto const get_neighbours = [&og](auto&& node_id) -> auto const& {
    return og.nodes_[node_id].out_;
  };

  utls::bfs(start, get_neighbours, work_on_node);

  return reachable;
}

std::vector<bool> get_exclusions(interlocking_route::id const ir_id,
                                 infrastructure const& infra) {
  std::vector<bool> exclusions(infra->interlocking_.routes_.size(), false);

  for (auto const set_id : infra->exclusion_.irs_to_exclusion_sets_[ir_id]) {
    for (auto const& excluded_ir : infra->exclusion_.exclusion_sets_[set_id]) {
      exclusions[excluded_ir] = true;
    }
  }

  return exclusions;
}

bool has_exclusion_paths(ordering_graph const& og, ordering_node const& start,
                         infrastructure const& infra) {
  auto const reachable = get_reachable_nodes(og, start.id_);

  auto const ir1 = infra->interlocking_.routes_[start.ir_id_];
  auto const exclusions = get_exclusions(ir1.id_, infra);

  auto const has_path_impl = [&](auto&& target) {
    if (start.id_ == target.id_) {
      return true;
    }

    auto const ir2 = infra->interlocking_.routes_[target.ir_id_];

    // ir1 in exclusion with ir2
    // => there must be an path from start to target
    if (exclusions[ir2.id_]) {
      bool const start_target_path = reachable[target.id_];
      bool const target_start_path =
          start_target_path || path_exists(og, target.id_, start.id_);
      bool const path_exists = start_target_path || target_start_path;
      if (!path_exists) {
        return false;
      }
    }

    return true;
  };

  return utls::all_of(og.nodes_, has_path_impl);
}

bool has_exclusion_paths(ordering_graph const& og,
                         infrastructure const& infra) {
  return utls::all_of(og.nodes_, [&](auto&& node) {
    return has_exclusion_paths(og, node, infra);
  });
}

}  // namespace soro::simulation
