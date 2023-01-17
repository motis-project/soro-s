#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"
#include "soro/utls/unixtime.h"

namespace soro::simulation {

struct ordering_node {
  using id = std::uint32_t;
  static constexpr id INVALID = std::numeric_limits<id>::max();

  ordering_node(id const id, infra::interlocking_route::id const ir_id,
                tt::train::id const train_id)
      : id_{id}, ir_id_{ir_id}, train_id_{train_id} {}

  id id_{INVALID};
  infra::interlocking_route::id ir_id_;
  tt::train::id train_id_;

  std::vector<id> in_;
  std::vector<id> out_;
};

struct ordering_graph {
  ordering_graph(infra::infrastructure const& infra, tt::timetable const& tt);
  ordering_graph();
  std::vector<ordering_node> nodes_;
};

ordering_graph generate_testgraph(int train_amnt, int track_amnt, int min_nodes,
                                  int max_nodes);

}  // namespace soro::simulation
