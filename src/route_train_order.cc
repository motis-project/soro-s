#include "soro/route_train_order.h"

#include <iostream>

#include "cista/containers/hash_map.h"
#include "cista/containers/hash_set.h"
#include "cista/containers/variant.h"

#include "utl/enumerate.h"
#include "utl/pairwise.h"

#include "soro/network.h"

namespace cr = cista::raw;

namespace soro {

template <typename Vec, typename BinaryPredicate, typename... Args>
void emplace_unique_sorted(Vec& v, BinaryPredicate&& cmp, Args&&... args) {
  using std::begin;
  using std::end;
  typename Vec::value_type val{std::forward<Args>(args)...};
  auto const it = std::lower_bound(begin(v), end(v), val, cmp);
  if (it != end(v) && val == *it) {
    return;
  }
  v.emplace(it, std::move(val));
}

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
          emplace_unique_sorted(route_nodes[e->from_], cmp, r.get());
          emplace_unique_sorted(route_nodes[e->to_], cmp, r.get());
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
          auto const r2_main = r2->main_ != nullptr ? r2->main_ : r2;
          r1->succ_->out_.emplace(r2_main);
          r2_main->in_.emplace(r1->succ_);
        }
      }
    }
  }

  // Remove transitive edges.
  // Example: (A->C) not necessary if (A->B) and (B->C) exist.
  for (auto const& [train_name, train] : tt) {
    for (auto& r : train->routes_) {
      for (auto out_it = begin(r->out_); out_it != end(r->out_);) {
        if (r->train_ != (*out_it)->train_ &&
            connected_transitively(r.get(), *out_it)) {
          (*out_it)->in_.erase(r.get());
          r->out_.erase(out_it++);
        } else {
          ++out_it;
        }
      }
    }
  }
}

}  // namespace soro