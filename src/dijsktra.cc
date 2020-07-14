#include "soro/dijkstra.h"

#include <iostream>
#include <queue>

#include "utl/verify.h"

namespace cr = cista::raw;

namespace soro {

std::vector<edge*> dijkstra(network const&, node* from, node* to) {
  struct queue_entry {
    bool operator>(queue_entry const& o) const { return dist_ > o.dist_; }
    std::vector<edge*> pred_;
    node* node_{nullptr};
    edge* prev_edge_{nullptr};
    unsigned dist_{0U};
  };
  std::priority_queue<queue_entry, std::vector<queue_entry>, std::greater<>> q;
  q.emplace(queue_entry{std::vector<edge*>{}, from});

  cr::hash_map<node*, cr::hash_map<edge*, unsigned>> dist;
  while (!q.empty()) {
    auto const curr = q.top();
    q.pop();

    if (curr.node_ == to) {
      return curr.pred_;
    }

    if (auto const node_it = dist.find(curr.node_); node_it != end(dist)) {
      if (auto const edge_it = node_it->second.find(curr.prev_edge_);
          edge_it != end(node_it->second) && edge_it->second < curr.dist_) {
        continue;
      }
    }

    dist[curr.node_][curr.prev_edge_] = curr.dist_;

    auto const expand_edges = [&](auto const& edges) {
      for (auto const& edge : edges) {
        if (edge == nullptr || edge->opposite(curr.node_) == nullptr) {
          continue;
        }
        queue_entry next;
        next.pred_ = curr.pred_;
        next.pred_.emplace_back(edge);
        next.prev_edge_ = edge;
        next.node_ = edge->opposite(curr.node_);
        next.dist_ = curr.dist_;
        next.dist_ += edge->draw_representation_.size();
        next.dist_ += curr.node_->draw_representation_.size();
        auto const diagonal_penalty = std::count_if(
            begin(edge->draw_representation_), end(edge->draw_representation_),
            [](auto&& c) { return c.content_ == '\\' || c.content_ == '/'; });
        next.dist_ += diagonal_penalty;
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

  return dijkstra(net, from_it->get(), to_it->get());
}

}  // namespace soro