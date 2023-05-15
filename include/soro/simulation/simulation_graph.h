#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/simulation/ordering_graph.h"
#include "soro/timetable/timetable.h"

namespace soro::simulation {

struct graph {
  graph(ordering_graph const& og, tt::timetable const& timetable,
        infra::infrastructure const& infra);

  struct node {
    using id = uint32_t;
    using idx = uint32_t;

    infra::node::ptr node_;
    tt::sequence_point::optional_ptr sequence_point_;
  };

  struct train {
    std::vector<node> nodes_;

    soro::fws_multimap<node::id, node::idx> in_;
    soro::fws_multimap<node::id, node::idx> out_;
  };
  
  std::vector<train> trains_;
};

}  // namespace soro::simulation