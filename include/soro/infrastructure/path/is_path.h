#pragma once

#include "soro/utls/concepts/iterable_yields.h"
#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/graph/graph.h"

namespace soro::infra {

inline bool neighbours(element_ptr e1, element_ptr e2) {
  return utls::any_of(e1->neighbours(),
                      [&](auto&& neigh) { return neigh->id() == e2->id(); });
}

template <typename Iterable>
  requires utls::yields<element_ptr, Iterable> bool
is_path(Iterable&& iterable) {
  element_ptr last_element = *std::cbegin(iterable);

  for (auto it = ++std::cbegin(iterable); it != std::cend(iterable); ++it) {
    if (!neighbours(last_element, *it)) {
      return false;
    }

    last_element = *it;
  }

  return true;
}

template <typename Iterable>
  requires utls::yields<node_ptr, Iterable> bool
is_path(Iterable&& iterable) {
  return is_path(
      utls::coro_map(iterable, [](auto&& n) { return n->element_; }));
}

}  // namespace soro::infra