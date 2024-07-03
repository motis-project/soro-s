#pragma once

#include "soro/utls/std_wrapper/any_of.h"

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

inline bool are_neighbours(element::ptr const e1, element::ptr const e2) {
  return utls::any_of(e1->neighbours(), [&](auto&& neigh) {
    return neigh->get_id() == e2->get_id();
  });
}

template <typename Element>
  requires(!is_pointer_v<Element>)
inline bool are_neighbours(Element const& e, element::ptr e2) {
  return utls::any_of(e.neighbours(), [&](auto&& neigh) {
    return neigh->get_id() == e2->get_id();
  });
}

inline bool are_neighbours(node::ptr const n1, node::ptr const n2) {
  return n1->next_ == n2 || n1->branch_ == n2 ||
         utls::any_of(n1->reverse_edges_, [&](auto&& n) { return n == n2; });
}

}  // namespace soro::infra
