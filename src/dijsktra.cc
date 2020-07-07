#include "rapid/dijkstra.h"

#include <iostream>
#include <queue>

#include "utl/verify.h"

namespace rapid {

std::vector<edge*> dijkstra(node* from, node* to) {
  struct queue_entry {
    bool operator<(queue_entry const& o) const { return dist_ < o.dist_; }
    std::vector<edge*> pred_;
    node* node_{nullptr};
    unsigned dist_{0U};
  };
  std::priority_queue<queue_entry> q;
  q.emplace(queue_entry{std::vector<edge*>{}, from, 0});

  cista::raw::hash_map<node*, unsigned> dist_;
  while (!q.empty()) {
    auto const curr = q.top();
    q.pop();

    if (curr.node_ == to) {
      return curr.pred_;
    }

    if (auto const it = dist_.find(curr.node_);
        it == end(dist_) || it->second > curr.dist_) {
      dist_[curr.node_] = curr.dist_;
    } else {
      continue;
    }

    auto const expand_edges = [&](auto const& edges) {
      for (auto const& edge : edges) {
        if (edge == nullptr || edge->opposite(curr.node_) == nullptr) {
          continue;
        }
        queue_entry next;
        next.pred_ = curr.pred_;
        next.pred_.emplace_back(edge);
        next.node_ = edge->opposite(curr.node_);
        next.dist_ += edge->dist_;
        q.emplace(next);
      }
    };

    if (curr.pred_.empty()) {
      expand_edges(std::initializer_list<edge*>{curr.node_->end_node_edge_});
      for (auto const& [from, to] : curr.node_->traversals_) {
        expand_edges(to);
      }
    } else if (auto const it = curr.node_->traversals_.find(curr.pred_.back());
               it != end(curr.node_->traversals_)) {
      expand_edges(it->second);
    }
  }
  return {};
}

std::vector<edge*> dijkstra(network const& net, std::string_view from,
                            std::string_view to) {
  auto const from_it = std::find_if(begin(net.nodes_), end(net.nodes_),
                                    [&](auto&& n) { return n->name_ == from; });
  auto const to_it = std::find_if(begin(net.nodes_), end(net.nodes_),
                                  [&](auto&& n) { return n->name_ == to; });

  utl::verify(from_it != end(net.nodes_), "dijkstra: from node {} not found",
              from);
  utl::verify(to_it != end(net.nodes_), "dijstra: to node {} not found", to);

  return dijkstra(from_it->get(), to_it->get());
}

}  // namespace rapid