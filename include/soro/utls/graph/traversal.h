#pragma once

#include <queue>
#include <stack>

#include "cista/containers/hash_set.h"

namespace soro::utls {

template <typename Node, typename HandleNodeFn, typename GetNeigh,
          typename Todo>
void graph_traversal(Node root, HandleNodeFn const& handle_node,
                     GetNeigh const& get_neighbours) {
  auto const get_current = [](Todo const& todo) {
    if constexpr (std::is_same_v<Todo, std::queue<std::pair<Node, Node>>>) {
      return todo.front();
    } else {
      return todo.top();
    }
  };

  Todo q;
  cista::raw::hash_set<Node> seen;

  q.emplace(std::pair{root, root});

  while (!q.empty()) {
    auto const current = get_current(q);
    auto const& current_node = current.first;
    auto const& prev_node = current.second;

    q.pop();

    if (handle_node(current_node, prev_node)) {
      return;
    }

    for (auto const& neighbour : get_neighbours(current_node)) {
      if (seen.find(neighbour) == std::end(seen)) {
        seen.emplace(neighbour);
        q.emplace(std::pair{neighbour, current_node});
      }
    }
  }
}

// To use bfs or dfs supply the following:
//
// - bool handle_node(Node, Node):
//  - Every node found during the traversal is handed to this function as the
//  first parameter.
//  - The node from which the first parameter node has been
//  reached is passed as a second parameter.
//  - If this function returns true the  traversal is terminated.
//
// - Container get_neigbhours(Node):
//  - Returns the neighbours of the node in an iterable container

template <typename Node, typename HandleNodeFn, typename GetNeigh>
void inline bfs(Node root, HandleNodeFn&& handle_node,
                GetNeigh&& get_neighbours) {
  return graph_traversal<Node, HandleNodeFn, GetNeigh,
                         std::queue<std::pair<Node, Node>>>(root, handle_node,
                                                            get_neighbours);
}

template <typename Node, typename HandleNodeFn, typename GetNeigh>
void inline dfs(Node root, HandleNodeFn&& handle_node,
                GetNeigh&& get_neighbours) {
  return graph_traversal<
      Node, HandleNodeFn, GetNeigh,
      std::stack<std::pair<Node, Node>, std::vector<std::pair<Node, Node>>>>(
      root, handle_node, get_neighbours);
}

}  // namespace soro::utls