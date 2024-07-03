#pragma once

#include "soro/base/soro_types.h"

#include "soro/utls/graph/traversal.h"
#include "soro/utls/parallel_for.h"
#include "soro/utls/print_progress.h"
#include "soro/utls/std_wrapper/min_element.h"
#include "soro/utls/std_wrapper/reverse.h"

namespace soro::test {

template <typename NodeId>
using cycle = std::vector<NodeId>;

namespace detail {

template <typename NodeId, typename Graph>
std::optional<cycle<NodeId>> get_shortest_cycle_from_node(NodeId const& n,
                                                          Graph const& g) {
  using depth_t = uint32_t;

  soro::vector_map<NodeId, uint32_t> depth(g.nodes_.size(),
                                           std::numeric_limits<depth_t>::max());
  soro::vector_map<NodeId, NodeId> prev(g.nodes_.size(), NodeId::invalid());

  depth[n] = 0;

  auto const work_on_node = [&depth, &prev](auto&& to, auto&& from) {
    if (depth[to] == std::numeric_limits<depth_t>::max()) {
      depth[to] = depth[from] + 1;
      prev[to] = from;
    }

    return false;
  };

  auto const work_on_edge = [&n, &prev](auto&& from, auto&& to) {
    if (to == n) {
      prev[to] = from;
      return true;
    }

    return false;
  };

  auto const get_neighbours = [&](auto&& node_id) { return g.out(node_id); };

  utls::bfs(n, get_neighbours, work_on_node, work_on_edge);

  bool const found_cycle = prev[n] != NodeId::invalid();
  if (!found_cycle) return std::nullopt;

  cycle<NodeId> c = {n};
  auto current = prev[n];
  while (current != n) {
    c.emplace_back(current);
    current = prev[current];
  }

  utls::reverse(c);

  return c;
}

template <typename Graph>
auto get_cycles(Graph const& g) {
  using NodeId = typename Graph::node::id;

  std::vector<cycle<NodeId>> result;

  std::mutex m;
  auto const do_work = [&](soro::size_t const& work_id) {
    utls::print_progress("checking for cycles in simulation graph", g.nodes_);

    auto cycle = get_shortest_cycle_from_node(NodeId{work_id}, g);

    if (cycle) {
      std::lock_guard const lock(m);
      result.emplace_back(std::move(*cycle));
    }
  };

  utls::parallel_for(g.nodes_.size(), do_work);

  return result;
}

template <typename Graph>
std::optional<cycle<typename Graph::node::id>> get_cycle(Graph const& g) {
  auto const cycles = detail::get_cycles(g);

  if (cycles.empty()) return std::nullopt;

  auto const shortest_cycle = utls::min_element(
      cycles, [](auto&& lhs, auto&& rhs) { return lhs.size() < rhs.size(); });

  return std::optional<cycle<typename Graph::node::id>>{*shortest_cycle};
}

}  // namespace detail

template <typename Graph>
[[nodiscard]] auto get_cycles(Graph const& g) {
  return detail::get_cycles(g);
}

template <typename Graph>
[[nodiscard]] auto get_cycle(Graph const& g) {
  return detail::get_cycle(g);
}

}  // namespace soro::test