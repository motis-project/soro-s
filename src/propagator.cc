#include "rapid/propagator.h"

#include "cista/containers/hash_set.h"

namespace cr = cista::raw;

namespace rapid {

void propagate(train_order_map const& route_train_order) {
  // Init queue.
  cr::hash_set<route*> todo;
  for (auto const& [fromto, routes] : route_train_order) {
    for (auto const& r : routes) {
      if (r->finished()) {
        continue;
      } else {
        if (r->ready()) {
          todo.emplace(r);
        }
      }
    }
  }

  // Propagate.
  while (!todo.empty()) {
    auto const next = *begin(todo);
    todo.erase(begin(todo));
    next->compute_dists();
    for (auto const& out : next->out_) {
      if (out->ready()) {
        todo.emplace(out);
      }
    }
  }
}

}  // namespace rapid