#pragma once

#include "cista/containers/hash_set.h"

#include "rapid/network.h"

namespace rapid {

template <typename Fn>
cista::raw::hash_set<node*> bfs_find_nodes(node* start, Fn&& predicate) {
  cista::raw::hash_set<std::pair<node*, edge*>> todo{{start, nullptr}}, done;
  cista::raw::hash_set<node*> found;
  while (!todo.empty()) {
    auto const [curr_node, prev_edge] = *todo.begin();
    todo.erase(todo.begin());
    if (predicate(curr_node)) {
      found.emplace(curr_node);
      continue;
    }
    for (auto const& next_edge : curr_node->traversals_[prev_edge]) {
      if (done.emplace(next_edge->opposite(curr_node), next_edge).second) {
        todo.emplace(next_edge->opposite(curr_node), next_edge);
      }
    }
  }
  return found;
}

}  // namespace rapid