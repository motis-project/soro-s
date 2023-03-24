#pragma once

#include "soro/utls/concepts/yields.h"
#include "soro/utls/coroutine/coro_map.h"
#include "soro/utls/std_wrapper/any_of.h"

#include "soro/infrastructure/graph/graph.h"

namespace soro::infra {

inline bool neighbours(element::ptr e1, element::ptr e2) {
  return utls::any_of(e1->neighbours(),
                      [&](auto&& neigh) { return neigh->id() == e2->id(); });
}

template <typename Iterable>
  requires utls::yields<element::ptr, Iterable>
inline bool is_path(Iterable&& iterable) {
  auto it = std::begin(iterable);

  element::ptr last_element = *it;
  ++it;

  for (; it != std::end(iterable); ++it) {
    if (!neighbours(last_element, *it)) {
      return false;
    }

    last_element = *it;
  }

  return true;
}

template <typename Iterable>
  requires utls::yields<node::ptr, Iterable>
inline bool is_path(Iterable&& iterable) {
  return is_path(
      utls::coro_map(iterable, [](auto&& n) { return n->element_; }));
}

}  // namespace soro::infra