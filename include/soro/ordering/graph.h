#pragma once

#include <limits>

#include "soro/base/soro_types.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

namespace soro::ordering {

namespace detail {

// basically a fwd declaration
using node_id = soro::strong<uint32_t, struct _ordering_graph_node_id>;

}  // namespace detail

struct graph {
  struct node;
  struct trip_group;

  using nodes_t = soro::vector_map<detail::node_id, node>;
  using edges_t = soro::vecvec<detail::node_id, detail::node_id>;

  struct node {
    using id = detail::node_id;

    static id invalid() { return std::numeric_limits<id>::max(); }

    id get_id(graph const& og) const;

    bool has_next(graph const& og) const;
    id next_id(graph const& og) const;
    node const& next(graph const& og) const;

    bool has_prev(graph const& og) const;
    id prev_id(graph const& og) const;
    node const& prev(graph const& og) const;

    bool is_first_in_trip(graph const& og) const;
    bool is_last_in_trip(graph const& og) const;

    edges_t::const_bucket in(graph const& og) const;
    edges_t::const_bucket out(graph const& og) const;

    trip_group get_trip_group(graph const& og) const;
    tt::train::id get_train_id(graph const& og) const;

    std::string print(graph const& og) const;

    infra::interlocking_route::id ir_id_{infra::interlocking_route::invalid()};
    tt::train::trip::id trip_id_{tt::train::trip::invalid()};
  };

  struct edge {
    bool operator==(edge const& other) const = default;
    node::id from_;
    node::id to_;
  };

  struct trip_group : tt::train::trip {
    std::span<node const> nodes(graph const& og) const;
    bool ok() const;

    node::id from_{node::invalid()};
    node::id to_{node::invalid()};
  };

  struct filter {
    bool filtered(tt::train const& train) const;

    tt::interval interval_;
    std::optional<std::vector<tt::train::id>> include_trains_;
    std::optional<std::vector<tt::train::id>> exclude_trains_;
  };

  graph() = default;
  graph(infra::infrastructure const& infra, tt::timetable const& tt);
  graph(infra::infrastructure const& infra, tt::timetable const& tt,
        filter const& filter);

  edges_t::const_bucket out(node::id const id) const;
  edges_t::const_bucket in(node::id const id) const;

  nodes_t nodes_;

  edges_t incoming_edges_;
  edges_t outgoing_edges_;

  soro::vector_map<tt::train::trip::id, trip_group> trips_;
};

// debug purposes only ...
// otherwise the gdb cannot construct a span for pretty printing a vecvec
// TODO(julian) find a different solution to the problem
inline std::span<const graph::node::id> get_my_span(graph::node::id const* data,
                                                    std::size_t size) {
  return {data, size};
}

}  // namespace soro::ordering
