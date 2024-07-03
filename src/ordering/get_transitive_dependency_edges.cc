#include "soro/ordering/get_transitive_dependency_edges.h"

#include <cstdint>

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/filter.hpp"
#include "range/v3/view/transform.hpp"

#include "soro/base/soro_types.h"

#include "soro/utls/graph/traversal.h"
#include "soro/utls/narrow.h"
#include "soro/utls/std_wrapper/any_of.h"

#include "soro/ordering/graph.h"

namespace soro::ordering {

soro::vector<graph::edge> get_transitive_dependency_edges(
    graph::node::id const start,
    soro::vector<soro::vector<graph::node::id>> const& outgoing,
    graph const& og) {
  using namespace ranges;

  // test only dependency edges for transitivity
  auto const consider_edge = [&](graph::node::id const from,
                                 graph::node::id const to) {
    return og.nodes_[from].get_trip_group(og).train_id_ !=
           og.nodes_[to].get_trip_group(og).train_id_;
  };

  auto const consider_any_edge = utls::any_of(
      outgoing[start.v_], [&](auto&& to) { return consider_edge(start, to); });

  if (!consider_any_edge) return {};

  soro::vector_map<graph::node::id, uint32_t> max_depth(
      utls::narrow<soro::size_t>(outgoing.size()), 0);

  max_depth[start] = 0;

  auto const work_on_node = [&max_depth](auto&& to, auto&& from) {
    max_depth[to] = std::max(max_depth[to], max_depth[from] + 1);
    return false;
  };

  auto const work_on_edge = [&max_depth](auto&& from, auto&& to) {
    max_depth[to] = std::max(max_depth[to], max_depth[from] + 1);
    return false;
  };

  auto const get_neighbours = [&outgoing](auto&& node_id) {
    return outgoing[node_id.v_];
  };

  utls::dfs(start, get_neighbours, work_on_node, work_on_edge);

  return outgoing[start.v_] |
         views::filter([&](auto&& to) { return consider_edge(start, to); }) |
         views::filter([&](auto&& to) { return max_depth[to] > 1; }) |
         views::transform([&](auto&& to) -> graph::edge {
           return {.from_ = start, .to_ = to};
         }) |
         ranges::to<soro::vector<graph::edge>>();
}

}  // namespace soro::ordering
