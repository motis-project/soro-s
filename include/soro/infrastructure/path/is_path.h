#pragma once

#include "soro/utls/concepts/yields.h"
#include "soro/utls/coroutine/coro_map.h"
#include "soro/utls/std_wrapper/any_of.h"

#include "soro/infrastructure/graph/are_neighbours.h"
#include "soro/infrastructure/graph/graph.h"

namespace soro::infra {

template <typename Iterable>
  requires utls::yields<element::ptr, Iterable>
inline bool is_path(Iterable&& iterable) {
  auto it = std::begin(iterable);

  element::ptr last_element = *it;
  ++it;

  for (; it != std::end(iterable); ++it) {
    if (!are_neighbours(last_element, *it)) {
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