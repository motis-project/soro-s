#pragma once

#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

template <typename Type>
element* create_element_t(graph& network, station& station,
                          construction_materials& mats, type const type,
                          soro::optional<rail_plan_node_id> const rp_id) {
  auto unique_element = soro::make_unique<struct element>();
  auto element = unique_element.get();

  element->e_ = Type{};

  auto& typed_element = element->e_.template as<Type>();

  typed_element.type_ = type;
  typed_element.id_ = static_cast<element_id>(network.elements_.size());

  for (std::size_t idx = 0; idx < typed_element.nodes_.size(); ++idx) {
    auto unique_node = soro::make_unique<infra::node>();
    auto node = unique_node.get();

    node->id_ = static_cast<node::id>(network.nodes_.size());

    node->element_ = element;

    typed_element.nodes_[idx] = node;

    network.nodes_.push_back(node);
    network.node_store_.emplace_back(std::move(unique_node));
    network.node_to_rp_id_.emplace_back(rp_id);
  }

  if (rp_id.has_value()) {
    mats.to_element_id_.add(*rp_id, element->get_id());
  }

  network.element_data_.emplace_back(empty{});
  network.element_to_rp_id_.emplace_back(rp_id);

  station.elements_.push_back(static_cast<element::ptr>(element));

  network.elements_.push_back(element);
  network.element_store_.emplace_back(std::move(unique_element));
  return element;
}

element* create_element(graph& network, station& station,
                        construction_materials& mats, type const type,
                        rail_plan_node_id const rp_id);

element* get_or_create_element(graph& network, station& station,
                               construction_materials& mats, type const type,
                               rail_plan_node_id const rp_id);

}  // namespace soro::infra
