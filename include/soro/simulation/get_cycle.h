#pragma once

#include "soro/utls/graph/traversal.h"
#include "soro/utls/std_wrapper/min_element.h"
#include "soro/utls/std_wrapper/reverse.h"

#include "soro/simulation/ordering_graph.h"

namespace soro::simulation {

using cycle = std::vector<ordering_node::id>;

namespace detail {

inline std::optional<cycle> get_shortest_cycle_from_node(
    ordering_node::id const start, ordering_graph const& og) {
  using depth_t = uint32_t;

  std::vector<uint32_t> depth(og.nodes_.size(),
                              std::numeric_limits<depth_t>::max());
  std::vector<ordering_node::id> prev(og.nodes_.size(), ordering_node::INVALID);

  depth[start] = 0;

  auto const work_on_node = [&depth, &prev](auto&& to, auto&& from) {
    if (depth[to] == std::numeric_limits<depth_t>::max()) {
      depth[to] = depth[from] + 1;
      prev[to] = from;
    }

    return false;
  };

  auto const work_on_edge = [&start, &prev](auto&& from, auto&& to) {
    if (to == start) {
      prev[to] = from;
      return true;
    }

    return false;
  };

  auto const get_neighbours = [&](auto&& node_id) -> auto const& {
    return og.nodes_[node_id].out_;
  };

  utls::bfs(start, get_neighbours, work_on_node, work_on_edge);

  bool const found_cycle = prev[start] != ordering_node::INVALID;
  if (!found_cycle) {
    return std::nullopt;
  }

  cycle c = {start};
  auto current = prev[start];
  while (current != start) {
    c.emplace_back(current);
    current = prev[current];
  }

  utls::reverse(c);

  return c;
}

}  // namespace detail

inline std::optional<cycle> get_cycle(ordering_graph const& og) {
  std::vector<cycle> cycles;

  for (auto const& start : og.nodes_) {
    auto cycle = detail::get_shortest_cycle_from_node(start.id_, og);

    if (cycle) {
      cycles.emplace_back(std::move(*cycle));
    }
  }

  if (cycles.empty()) {
    return std::nullopt;
  }

  return *utls::min_element(
      cycles, [](auto&& lhs, auto&& rhs) { return lhs.size() < rhs.size(); });
}

}  // namespace soro::simulation