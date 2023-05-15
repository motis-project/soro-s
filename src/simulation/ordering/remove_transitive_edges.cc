#include "soro/simulation/ordering/remove_transitive_edges.h"

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/filter.hpp"
#include "range/v3/view/transform.hpp"

#include "utl/concat.h"
#include "utl/erase.h"
#include "utl/timer.h"

#include "soro/utls/graph/traversal.h"

namespace soro::simulation {

std::vector<ordering_edge> get_transitive_edges(ordering_graph const& og,
                                                ordering_node const& start) {
  std::vector<uint32_t> max_depth(og.nodes_.size(), 0);

  max_depth[start.id_] = 0;

  auto const work_on_node = [&max_depth](auto&& to, auto&& from) {
    max_depth[to] = std::max(max_depth[to], max_depth[from] + 1);
    return false;
  };

  auto const work_on_edge = [&max_depth](auto&& from, auto&& to) {
    max_depth[to] = std::max(max_depth[to], max_depth[from] + 1);
    return false;
  };

  auto const get_neighbours = [&og](auto&& node_id) -> auto const& {
    return og.nodes_[node_id].out_;
  };

  utls::dfs(start.id_, get_neighbours, work_on_node, work_on_edge);

  return start.out_ | ranges::views::filter([&max_depth](auto&& to) {
           return max_depth[to] > 1;
         }) |
         ranges::views::transform(
             [&start](auto&& to) { return std::make_pair(start.id_, to); }) |
         ranges::to<std::vector>();
}

std::vector<ordering_edge> get_transitive_edges(ordering_graph const& og) {
  std::vector<ordering_edge> transitive_edges;

  for (auto const& start : og.nodes_) {
    utl::concat(transitive_edges, get_transitive_edges(og, start));
  }

  return transitive_edges;
}

void remove_transitive_edges(ordering_graph& og) {
  utl::scoped_timer const timer("removing transitive edges");

  auto const transitive_edges = get_transitive_edges(og);

  for (auto const& edge : transitive_edges) {
    utl::erase(og.nodes_[edge.first].out_, edge.second);
    utl::erase(og.nodes_[edge.second].in_, edge.first);
  }
}

}  // namespace soro::simulation