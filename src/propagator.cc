#include "soro/propagator.h"

#include "cista/containers/hash_set.h"

#include "soro/timetable_parser.h"

namespace cr = cista::raw;

namespace soro {

void propagate(timetable const& tt) {
  // Init queue.
  cr::hash_set<route*> todo;
  for (auto const& [train_name, t] : tt) {
    for (auto const& r : t->routes_) {
      if (r->finished()) {
        continue;
      } else {
        if (r->ready()) {
          todo.emplace(r.get());
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

}  // namespace soro