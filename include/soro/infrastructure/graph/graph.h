#pragma once

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

struct graph {
  template <typename T>
  T const& element_data(element::ptr const e) const {
    return element_data_[e->id()].as<T>();
  }

  template <typename T>
  T const& element_data(node::ptr const n) const {
    return element_data<T>(n->element_);
  }

  soro::vector<node::ptr> nodes_;
  soro::vector<element::ptr> elements_;

  soro::vector<element_data_t> element_data_;

  soro::vector<section> sections_;
  soro::vector<soro::vector<section::id>> element_id_to_section_ids_;

  soro::vector<soro::unique_ptr<node>> node_store_;
  soro::vector<soro::unique_ptr<element>> element_store_;
};

}  // namespace soro::infra
