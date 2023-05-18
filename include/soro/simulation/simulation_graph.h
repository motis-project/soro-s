#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/simulation/ordering/ordering_graph.h"
#include "soro/timetable/timetable.h"

namespace soro::simulation {

struct simulation_graph {
  struct node {
    using id = uint32_t;

    id get_id(simulation_graph const& sg) const;

    infra::element_id element_id_{infra::element::INVALID};
  };

  using vecvec_idx = uint32_t;
  using ordering_group = std::span<node>;

  simulation_graph() = default;
  simulation_graph(infra::infrastructure const& infra,
                   tt::timetable const& timetable, ordering_graph const& og);

  // nodes layout for n trips:
  // [ trip 1 nodes , trip 2 nodes ... trip n nodes]
  // trip i nodes layout for m interlocking routes in trip i:
  // [ ir 1 nodes , ir 2 nodes ... ir m nodes]
  soro::vector<node> nodes_;
  soro::vecvec<vecvec_idx, node::id> in_;
  soro::vecvec<vecvec_idx, node::id> out_;

  // ordering graph node id indexed
  soro::vector<ordering_group> ordering_groups_;

  // span into nodes_, indexed with train id
  soro::vector<std::span<node>> trips_;
};

}  // namespace soro::simulation
