#pragma once

#include "soro/infrastructure/infrastructure.h"

#include "soro/runtime/runtime_calculator.h"

#include "soro/simulation/graph/graph.h"

namespace soro::sim {

struct simulator {
  struct sim_node_data {
    graph::node::id identity_{graph::node::id::invalid()};
    absolute_time arrival_{INVALID<absolute_time>};
    absolute_time departure_{INVALID<absolute_time>};
  };

  using results_t = soro::vector_map<graph::node::id, sim_node_data>;

  simulator(infra::infrastructure const& infra, tt::timetable const& tt,
            graph const& sg);

  void operator()(graph::simulation_group const& sg, graph const& g,
                  tt::timetable const& tt);

  std::span<sim_node_data> result_span(graph::simulation_group const& sg) {
    return std::span{std::begin(results_) + to_idx(sg.from_),
                     std::begin(results_) + to_idx(sg.to_)};
  }

  results_t results_;
  runtime::calculator calculator_;
};

}  // namespace soro::sim
