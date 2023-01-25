#pragma once

#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

template <typename Type>
element* create_element_t(graph& network, station& station,
                          construction_materials& mats, type const type,
                          rail_plan_node_id const rp_id, bool const rising) {
  auto unique_element = soro::make_unique<struct element>();
  auto element = unique_element.get();

  element->e_ = Type{};

  auto& typed_element = element->e_.template as<Type>();

  typed_element.type_ = type;
  typed_element.id_ = static_cast<element_id>(network.elements_.size());
  typed_element.rising_ = rising;

  for (std::size_t idx = 0; idx < typed_element.nodes_.size(); ++idx) {
    auto unique_node = soro::make_unique<infra::node>();
    auto node = unique_node.get();

    node->id_ = static_cast<node::id>(network.nodes_.size());

    node->element_ = element;

    typed_element.nodes_[idx] = node;

    network.nodes_.push_back(node);
    network.node_store_.emplace_back(std::move(unique_node));
  }

  mats.rp_id_to_element_id_[rp_id] = element->id();

  network.element_id_to_section_ids_.emplace_back();
  network.element_data_.emplace_back(empty{});

  station.elements_.push_back(static_cast<element_ptr>(element));

  network.elements_.push_back(element);
  network.element_store_.emplace_back(std::move(unique_element));
  return element;
}

element* create_element(graph& network, station& station,
                        construction_materials& mats, type const type,
                        rail_plan_node_id const rp_id, bool const rising);

element* get_or_create_element(graph& network, station& station,
                               construction_materials& mats, type const type,
                               rail_plan_node_id const rp_id,
                               bool const rising);

void set_km_point_and_line(element& e, std::string const& node_name,
                           kilometrage km_point, line::id line);

void set_neighbour(element& e, std::string const& name, element* neigh,
                   bool const rising);

void connect_nodes(graph& n);

section::id create_section(graph& n);

void connect_border(simple_element& from_border, bool low_border,
                    element_ptr to_border);

}  // namespace soro::infra
