#pragma once

#include "soro/utls/unixtime.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

namespace soro::simulation {

struct ordering_graph;

struct ordering_node {
  using id = uint32_t;
  static constexpr id INVALID = std::numeric_limits<id>::max();

  id get_id(ordering_graph const& og) const;

  bool has_next(ordering_graph const& og) const;
  ordering_node const& next(ordering_graph const& og) const;

  id id_{INVALID};
  infra::interlocking_route::id ir_id_{infra::interlocking_route::INVALID};
  tt::train::id train_id_{tt::train::INVALID};

  std::vector<id> in_;
  std::vector<id> out_;
};

using ordering_edge = std::pair<ordering_node::id, ordering_node::id>;

struct ordering_graph {
  struct filter {
    tt::interval interval_{};
    std::vector<tt::train::id> trains_{};
  };

  ordering_graph() = default;
  ordering_graph(infra::infrastructure const& infra, tt::timetable const& tt);
  ordering_graph(infra::infrastructure const& infra, tt::timetable const& tt,
                 filter const& filter);

  std::span<const ordering_node> trip_nodes(tt::train::trip const trip) const;

  std::vector<ordering_node> nodes_;

  soro::vector<soro::optional<ordering_node::id>> next_;
  soro::vector<soro::optional<ordering_node::id>> prev_;

  // TODO(julian) create trip ids for ordering graph
  // and use a vector instead of a map here?
  std::map<tt::train::trip, std::pair<ordering_node::id, ordering_node::id>>
      trip_to_nodes_;
};

}  // namespace soro::simulation
