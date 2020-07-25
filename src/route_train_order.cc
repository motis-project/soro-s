#include "soro/route_train_order.h"

#include <iostream>

#include "cista/containers/hash_map.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/variant.h"

#include "utl/emplace_sorted.h"
#include "utl/enumerate.h"
#include "utl/pairwise.h"

#include "soro/network.h"

namespace cr = cista::raw;

namespace soro {

bool connected_transitively(route const* source, route const* target) {
  cr::hash_set<route const*> q{source}, done;
  while (!q.empty()) {
    auto curr = *q.begin();
    q.erase(q.begin());
    for (auto const& out : curr->out_) {
      if (out == target &&
          /* looking for transitive, not direct connections */ curr != source) {
        return true;
      } else if (done.find(out) == end(done)) {
        q.emplace(out);
        done.emplace(out);
      }
    }
  }
  return false;
}

void compute_route_train_order(timetable const& tt) {
  // Create mapping (node -> routes that use this node ordered by time).
  cr::hash_map<node const*, std::vector<route*>> route_nodes;
  auto const cmp = [](route const* a, route const* b) {
    return a->from_time_ < b->from_time_;
  };
  for (auto& [name, t] : tt) {
    for (auto const& r : t->routes_) {
      for (auto const [i, e] : utl::enumerate(r->path_)) {
        if (i != 0) {
          utl::emplace_sorted(route_nodes[e->from_], cmp, r.get());
          utl::emplace_sorted(route_nodes[e->to_], cmp, r.get());
        }
      }
    }
  }

  // Build dependency edges for the order.
  train_order_map route_train_order;
  for (auto const& [node, route_order] : route_nodes) {
    for (auto i = begin(route_order); i != end(route_order); ++i) {
      auto const& r1 = *i;
      for (auto j = std::next(i); j != end(route_order); ++j) {
        auto const& r2 = *j;
        if (r1->succ_ != nullptr) {
          r1->succ_->out_.emplace(r2);
          r2->in_.emplace(r1->succ_);
        }
      }
    }
  }

  // Remove transitive edges.
  // Example: (A->C) not necessary if (A->B) and (B->C) exist.
  for (auto const& [train_name, train] : tt) {
    for (auto& r : train->routes_) {
      for (auto out_it = begin(r->out_); out_it != end(r->out_);) {
        if (connected_transitively(r.get(), *out_it)) {
          r->out_.erase(out_it++);
        } else {
          ++out_it;
        }
      }
    }
  }
}

}  // namespace soro