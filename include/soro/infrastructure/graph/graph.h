#pragma once

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

struct graph {
  template <typename T>
  T const& get_element_data(element::id const id) const {
    return element_data_[id].as<T>();
  }

  template <typename T>
  T const& get_element_data(element::ptr const e) const {
    return get_element_data<T>(e->get_id());
  }

  template <typename T>
  T const& get_element_data(node::ptr const n) const {
    return get_element_data<T>(n->element_);
  }

  soro::vector_map<node::id, node::ptr> nodes_;
  soro::vector_map<element::id, element::ptr> elements_;

  soro::vector_map<element::id, element_data> element_data_;

  sections sections_;

  soro::vector_map<element::id, soro::optional<uint64_t>> element_to_rp_id_;
  soro::vector_map<node::id, soro::optional<uint64_t>> node_to_rp_id_;

  soro::vector<soro::unique_ptr<node>> node_store_;
  soro::vector<soro::unique_ptr<element>> element_store_;
};

}  // namespace soro::infra
