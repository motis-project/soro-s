#pragma once

#include <limits>

#include "cista/strong.h"

#include "soro/infrastructure/graph/predicates.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/ordering/graph.h"
#include "soro/timetable/sequence_point.h"
#include "soro/timetable/timetable.h"

namespace soro::sim {

namespace detail {

using node_id = soro::strong<uint32_t, struct _simulation_graph_node_id>;

}  // namespace detail

struct graph {
  using timetable_dependency = tt::sequence_point::optional_ptr;
  using timetable_dependencies_t =
      soro::vector_map<detail::node_id, timetable_dependency>;

  using dependency_key = detail::node_id;
  using train_dependency = detail::node_id;
  using train_dependencies_t = soro::vecvec<dependency_key, train_dependency>;

  using train_dependent = train_dependency;
  using train_dependents_t = train_dependencies_t;

  struct interlocking_group;
  struct trip_group;
  struct simulation_group;

  struct node {
    using id = detail::node_id;
    using optional_id = soro::optional<id>;

    static id invalid();

    id get_id(graph const& g) const;

    interlocking_group const& get_interlocking_group(graph const& g) const;
    trip_group const& get_trip_group(graph const& g) const;
    simulation_group const& get_simulation_group(graph const& g) const;

    bool has_train_dependencies(graph const& g) const;
    train_dependencies_t::const_bucket in(graph const& sg) const;

    bool has_train_dependents(graph const& g) const;
    train_dependents_t::const_bucket out(graph const& sg) const;

    bool is_first_in_trip(graph const& g) const;
    bool is_last_in_trip(graph const& g) const;

    bool can_have_train_dependency(infra::infrastructure const& infra) const;

    infra::element::id element_id_{infra::element::invalid()};
    // TODO(julian) inferable with element_id_ and infrastructure
    infra::type type_{infra::type::INVALID};
  };

  struct group {
    using id = uint32_t;
    using offset = uint16_t;
    using opt_offset = soro::optional<offset>;

    node const& front(graph const& g) const;
    node const& back(graph const& g) const;
    node const& at(graph const& g, offset const idx) const;

    std::span<node const> nodes(graph const& g) const;

    offset size() const;
    bool empty() const;

    node::id from_{node::invalid()};
    node::id to_{node::invalid()};
  };

  struct interlocking_group : group {
    interlocking_group(node::id const from, node::id const to,
                       opt_offset const approach_signal, opt_offset const halt);

    bool has_halt() const;
    bool has_approach() const;
    bool approach_before_halt() const;

    std::span<node const> skip_entry_ms(graph const& g) const;

    // as a single interlocking route can contain multiple halts ...
    // we only save the last halt offset
    opt_offset last_halt_;
    opt_offset approach_signal_;
  };

  struct trip_group : group, tt::train::trip {
    using id = tt::train::trip::id;

    trip_group(node::id const from, node::id const to,
               tt::train::trip::id const trip_id, tt::train::id const train_id,
               absolute_time const anchor);

    id get_id(graph const& g) const;
  };

  struct simulation_group : group {
    using id = cista::strong<uint32_t, struct _simulation_group_id>;

    using train_dependencies_t = soro::vecvec<id, id>;
    using train_dependents_t = soro::vecvec<id, id>;

    static id invalid() { return id{std::numeric_limits<id::value_t>::max()}; }

    simulation_group(node::id const from, node::id const to);

    id get_id(graph const& g) const;

    trip_group::id const& get_trip_id(graph const& g) const;
    trip_group const& get_trip(graph const& g) const;

    bool has_previous(graph const& g) const;
    id previous_id(graph const& g) const;
    simulation_group const& previous(graph const& g) const;

    bool has_next(graph const& g) const;
    id next_id(graph const& g) const;
    simulation_group const& next(graph const& g) const;

    bool has_train_dependencies(graph const& g) const;
    train_dependencies_t::const_bucket get_train_dependencies(
        graph const& g) const;

    bool has_train_dependents(graph const& g) const;
    train_dependents_t::const_bucket get_train_dependents(graph const& g) const;
  };

  using simulation_groups_t =
      soro::vector_map<simulation_group::id, simulation_group>;

  graph(infra::infrastructure const& infra, tt::timetable const& timetable,
        ordering::graph const& og);

  static infra::type_set occuring_types();

  train_dependencies_t::const_bucket in(node::id const id) const;
  train_dependents_t::const_bucket out(node::id const id) const;

  // nodes layout for n trips:
  // [ trip 1 nodes , trip 2 nodes ... trip n nodes]
  // trip i nodes layout for m interlocking routes in trip i:
  // [ ir 1 nodes , ir 2 nodes ... ir m nodes]
  soro::vector_map<node::id, node> nodes_;

  timetable_dependencies_t timetable_dependencies_;
  train_dependencies_t train_dependencies_;  // incoming edges
  train_dependents_t train_dependents_;  // outgoing edges

  soro::vector_map<ordering::graph::node::id, interlocking_group>
      interlocking_groups_;
  soro::vector_map<node::id, interlocking_group::id>
      node_to_interlocking_group_;

  soro::vector_map<trip_group::id, trip_group> trips_;
  soro::vector_map<node::id, trip_group::id> node_to_trip_group_;

  simulation_groups_t simulation_groups_;
  // TODO(julian) this is not 100% correct, as a node can belong to two
  // simulation groups
  soro::vector_map<node::id, simulation_group::id> node_to_simulation_group_;

  // incoming edges
  simulation_group::train_dependencies_t simulation_group_dependencies_;
  // outgoing edges
  simulation_group::train_dependents_t simulation_group_dependents_;

  soro::vector_map<node::id, ordering::graph::node::id>
      simulation_node_to_ordering_node_;
};

// debug purposes only ...
// otherwise the gdb cannot construct a span for pretty printing a vecvec
// TODO(julian) find a different solution to the problem
inline std::span<graph::train_dependency> get_my_span(
    graph::train_dependency* data, std::size_t size) {
  return {data, size};
}

}  // namespace soro::sim
