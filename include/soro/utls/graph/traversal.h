#pragma once

#include <queue>
#include <stack>

#include "cista/containers/hash_set.h"

#include "soro/utls/concepts/invocable_returns.h"
#include "soro/utls/concepts/yields.h"

namespace soro::utls {

namespace detail {

template <typename Todo, typename Node, typename GetNeighbours,
          typename WorkOnNode, typename WorkOnEdge>
inline void graph_traversal(Node const start, GetNeighbours&& get_neighbours,
                            WorkOnNode&& work_on_node,
                            WorkOnEdge&& work_on_edge) {
  auto const get_current = [](Todo const& todo) {
    if constexpr (std::is_same_v<Todo, std::queue<std::pair<Node, Node>>>) {
      return todo.front();
    } else {
      return todo.top();
    }
  };

  Todo q;
  cista::raw::hash_set<Node> seen;

  q.emplace(std::pair{start, start});

  while (!q.empty()) {
    auto const current = get_current(q);
    auto const& current_node = current.first;
    auto const& prev_node = current.second;

    q.pop();

    if (current_node != start && work_on_node(current_node, prev_node)) {
      return;
    }

    for (auto const& neighbour : get_neighbours(current_node)) {
      if (work_on_edge(current_node, neighbour)) {
        return;
      }

      if (seen.find(neighbour) == std::end(seen)) {
        seen.emplace(neighbour);
        q.emplace(std::pair{neighbour, current_node});
      }
    }
  }
}

template <typename Node>
using bfs_container = std::queue<std::pair<Node, Node>>;

template <typename Node>
using dfs_container =
    std::stack<std::pair<Node, Node>, std::vector<std::pair<Node, Node>>>;

}  // namespace detail

template <typename Fn, typename Node>
concept get_neighbours = std::invocable<Fn, Node> &&
                         utls::yields<Node, std::invoke_result_t<Fn, Node>>;

// To use bfs or dfs supply the following:

// 1. Node start:
//    Object of type Node from which the traversal starts
//
// 2. Iterable get_neighbours(Node):
//    Returns the neighbours of the node as an iterable
//
// The following are optional, but at providing least one is required:
//
// 3. bool work_on_node(Node, Node):
//    Every node found during the traversal is handed to this function as the
//    first parameter. Every node is visited only once.
//
//    The node from which the first parameter node has been reached
//    is passed as a second parameter.
//
//    If this function returns true the  traversal is terminated.
//
// 4. bool work_on_edge(Node, Node):
//    Every edge found during the traversal is handed to this function as a
//    pair of nodes as the parameters. Every edge is visited exactly once,
//    even edges that lead to already visited nodes.

//    If this function returns true the  traversal is terminated.
//
//    No return value

template <typename Node, get_neighbours<Node> GetNeighbours,
          invocable_returns<bool, Node, Node> WorkOnNode,
          invocable_returns<bool, Node, Node> WorkOnEdge>
void inline bfs(Node const start, GetNeighbours&& get_neighbours,
                WorkOnNode&& work_on_node, WorkOnEdge&& work_on_edge) {
  detail::graph_traversal<detail::bfs_container<Node>>(
      start, std::forward<GetNeighbours>(get_neighbours),
      std::forward<WorkOnNode>(work_on_node),
      std::forward<WorkOnEdge>(work_on_edge));
}

template <typename Node, get_neighbours<Node> GetNeighbours,
          invocable_returns<bool, Node, Node> WorkOnNode>
void inline bfs(Node const start, GetNeighbours&& get_neighbours,
                WorkOnNode&& work_on_node) {
  detail::graph_traversal<detail::bfs_container<Node>>(
      start, std::forward<GetNeighbours>(get_neighbours),
      std::forward<WorkOnNode>(work_on_node),
      [](auto&&, auto&&) { return false; });
}

template <typename Node, get_neighbours<Node> GetNeighbours,
          invocable_returns<bool, Node, Node> WorkOnNode,
          invocable_returns<bool, Node, Node> WorkOnEdge>
void inline dfs(Node const start, GetNeighbours&& get_neighbours,
                WorkOnNode&& work_on_node, WorkOnEdge&& work_on_edge) {
  detail::graph_traversal<detail::dfs_container<Node>>(
      start, std::forward<GetNeighbours>(get_neighbours),
      std::forward<WorkOnNode>(work_on_node),
      std::forward<WorkOnEdge>(work_on_edge));
}

template <typename Node, get_neighbours<Node> GetNeighbours,
          invocable_returns<bool, Node, Node> WorkOnNode>
void inline dfs(Node start, GetNeighbours&& get_neighbours,
                WorkOnNode&& work_on_node) {
  detail::graph_traversal<detail::dfs_container<Node>>(
      start, std::forward<GetNeighbours>(get_neighbours),
      std::forward<WorkOnNode>(work_on_node),
      [](auto&&, auto&&) { return false; });
}

}  // namespace soro::utls