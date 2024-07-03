#include "soro/ordering/graph.h"

#include <cstddef>
#include <map>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/logging.h"
#include "utl/pairwise.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/narrow.h"
#include "soro/utls/parallel_for.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/is_sorted.h"
#include "soro/utls/std_wrapper/sort.h"

#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/runtime/common/use_surcharge.h"
#include "soro/runtime/rk4_runtime.h"

#include "soro/ordering/remove_transitive_edges.h"

namespace soro::ordering {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;

void print_ordering_graph_stats(graph const& og) {
  // edge count e -> node count with edge count e
  std::map<std::size_t, std::size_t> in_edge_counts;
  std::map<std::size_t, std::size_t> out_edge_counts;

  for (auto const& n : og.nodes_) {
    ++in_edge_counts[n.in(og).size()];
    ++out_edge_counts[n.out(og).size()];
  }

  uLOG(utl::info) << "ordering graph node count: " << og.nodes_.size();
  uLOG(utl::info) << "ordering graph edge count: "
                  << og.outgoing_edges_.data_.size();

  uLOG(utl::info) << "incoming edges distribution:";
  for (auto const& [edge_count, nodes] : in_edge_counts) {
    uLOG(utl::info) << "nodes with " << edge_count << " in edges: " << nodes;
  }

  uLOG(utl::info) << "outgoing edges distribution:";
  for (auto const& [edge_count, nodes] : out_edge_counts) {
    uLOG(utl::info) << "nodes with " << edge_count << " out edges: " << nodes;
  }

  uLOG(utl::info) << "Total trips in ordering graph: " << og.trips_.size();
}

struct route_usage {
  route_usage(absolute_time const from, absolute_time const to,
              train::id const train_id, train::trip::id const trip_id,
              absolute_time const trip_anchor,
              interlocking_route::id const ir_id)
      : from_{from},
        to_{to},
        id_{graph::node::invalid()},
        train_id_{train_id},
        trip_id_{trip_id},
        trip_anchor_{trip_anchor},
        ir_id_{ir_id} {}

  absolute_time from_;
  absolute_time to_;

  graph::node::id id_{graph::node::invalid()};

  train::id train_id_{train::invalid()};
  train::trip::id trip_id_{train::trip::invalid()};
  absolute_time trip_anchor_{absolute_time::max()};
  interlocking_route::id ir_id_{interlocking_route::invalid()};
};

using route_orderings = std::vector<std::vector<route_usage>>;

void sort_orderings(route_orderings& orderings) {
  utl::scoped_timer const timer("sort route orderings");

  utl::parallel_for(orderings, [](auto&& usage_order) {
    utls::sort(usage_order, [](auto&& usage1, auto&& usage2) {
      return usage1.from_ < usage2.from_;
    });
  });
}

struct get_nodes_result {
  graph::nodes_t nodes_;
  soro::vector_map<tt::train::trip::id, graph::trip_group> trips_;
};

get_nodes_result get_nodes(soro::vector<route_usage> const& route_usages) {
  utl::scoped_timer const timer("creating ordering graph nodes from usages");

  get_nodes_result result;

  if (route_usages.empty()) return result;

  result.nodes_.resize(
      utls::narrow<graph::nodes_t::size_type>(route_usages.size()));
  result.trips_.resize(
      route_usages.back().trip_id_.v_ + 1,
      graph::trip_group{train::trip{train::trip::invalid(), train::invalid(),
                                    INVALID<absolute_time>}});

  auto const is_first_in_trip = [&](soro::size_t const usage_idx) {
    return usage_idx == 0 || route_usages[usage_idx - 1].trip_id_ !=
                                 route_usages[usage_idx].trip_id_;
  };

  auto const is_last_in_trip = [&](soro::size_t const usage_idx) {
    return usage_idx == route_usages.size() - 1 ||
           route_usages[usage_idx].trip_id_ !=
               route_usages[usage_idx + 1].trip_id_;
  };

  utls::parallel_for(result.nodes_.size(), [&](auto&& job_idx) {
    auto const& usage = route_usages[job_idx];
    auto const id = graph::node::id{job_idx};

    result.nodes_[id].ir_id_ = usage.ir_id_;
    result.nodes_[id].trip_id_ = usage.trip_id_;

    if (is_first_in_trip(job_idx)) {
      result.trips_[usage.trip_id_].id_ = usage.trip_id_;
      result.trips_[usage.trip_id_].from_ = id;
      result.trips_[usage.trip_id_].train_id_ = usage.train_id_;
      result.trips_[usage.trip_id_].anchor_ = usage.trip_anchor_;
    }

    if (is_last_in_trip(job_idx)) {
      result.trips_[usage.trip_id_].to_ = id + 1;
    }
  });

  utls::ensures([&] {
    // ensure consecutive trip ids at every node
    for (auto const [n1, n2] : utl::pairwise(result.nodes_)) {
      utls::ensure(n1.trip_id_ == n2.trip_id_ ||
                   n1.trip_id_ + 1 == n2.trip_id_);
    }

    // ensure consecutive trip ids
    for (auto const [t1, t2] : utl::pairwise(result.trips_)) {
      utls::ensure(t1.id_ + 1 == t2.id_);
    }

    // ensure trips partition the nodes
    utls::ensure(result.trips_.front().from_ == 0);
    for (auto const [t1, t2] : utl::pairwise(result.trips_)) {
      utls::ensure(t1.to_ == t2.from_);
    }
    utls::ensure(result.trips_.back().to_ == result.nodes_.size());

    for (auto const& t : result.trips_) {
      utls::ensure(t.ok());
      utls::ensure(result.nodes_[t.from_].trip_id_ ==
                   result.nodes_[t.to_ - 1].trip_id_);
    }
  });

  return result;
}

route_orderings get_route_orderings(
    soro::vector<route_usage> const& route_usages,
    infrastructure const& infra) {
  utl::scoped_timer const timer("generating route orderings");

  route_orderings orderings(infra->exclusion_.exclusion_sets_.size());

  for (auto const& usage : route_usages) {
    for (auto const es_id :
         infra->exclusion_.irs_to_exclusion_sets_[usage.ir_id_]) {
      orderings[es_id].push_back(usage);
    }
  }

  sort_orderings(orderings);

  return orderings;
}

soro::vector<route_usage> get_route_usages(graph::filter const& filter,
                                           timetable const& tt,
                                           infrastructure const& infra) {
  utl::scoped_timer const timer("generating route usages");

  auto const generate_route_usages = [&infra, &filter](train const& train) {
    soro::vector<route_usage> result;

    auto const trips_in_interval = train.trip_count(filter.interval_);

    if (filter.filtered(train) || trips_in_interval == 0) {
      return result;
    }

    std::vector<relative_time> times;

    // we will have two determine train.path_.size() route usages
    // therefore we need train.path_.size() + 1 timestamps
    times.reserve(train.path_.size() + 1);

    if (!train.break_in_ ||
        (train.break_in_ &&
         !train.get_start_node(infra)->is(type::MAIN_SIGNAL))) {
      times.emplace_back(train.start_time_);
    }

    auto recorded_approach = false;
    auto const record = [&](auto&& e) {
      if (e.element_->is(type::MAIN_SIGNAL) && recorded_approach) {
        recorded_approach = false;
        return;
      }

      if (e.element_->is(type::MAIN_SIGNAL) && !recorded_approach) {
        times.emplace_back(std::max(e.arrival_, e.departure_));
        return;
      }

      if (e.element_->is(type::APPROACH_SIGNAL) && !recorded_approach) {
        times.emplace_back(std::max(e.arrival_, e.departure_));
        recorded_approach = e.element_->is(type::APPROACH_SIGNAL);
      }
    };

    auto const last_timestamp = rk4::runtime_calculation(
        train, infra, {type::MAIN_SIGNAL}, use_surcharge::yes, record);

    if (recorded_approach) times.pop_back();

    if (!train.break_out_ ||
        (train.break_out_ &&
         !train.get_end_node(infra)->is(type::MAIN_SIGNAL))) {
      times.emplace_back(last_timestamp);
      //      times.emplace_back(train.last_timestamp());
    }

    utls::sassert(times.size() == train.path_.size() + 1,
                  "expected times size {}, but got {}", train.path_.size() + 1,
                  times.size());
    utls::sassert(utls::is_sorted(times), "timestamps not sorted");

    if (times.empty()) {
      uLOG(utl::warn) << "no main signal in path of train " << train.id_;
      return result;
    }

    train::trip::id trip_id{0};
    for (auto const anchor : train.departures(filter.interval_)) {
      auto path_idx = 0U;
      for (auto const [from_time, to_time] : utl::pairwise(times)) {
        result.emplace_back(relative_to_absolute(anchor, from_time),
                            relative_to_absolute(anchor, to_time), train.id_,
                            trip_id, anchor, train.path_[path_idx]);
        ++path_idx;
      }

      ++trip_id;
      utls::sassert(path_idx == train.path_.size(), "did not use up all IRs");
    }

    //    utls::sasserts([&]() {
    //      auto ms_count = utls::count_if(
    //          times, [](auto&& t) { return t.element_->is(type::MAIN_SIGNAL);
    //          });
    //
    //      utls::sassert(
    //          train.path_.size() == ms_count + 1,
    //          "Differing amounts of interlocking routes in train path and "
    //          "main signals in running time calculation timestamps");
    //    });

    //    train::trip::id trip_id{0};
    //
    //    result.reserve(trips_in_interval * train.path_.size());
    //    for (auto const anchor : train.departures(filter.interval_)) {
    //      // add first halt -> first ms
    //      result.emplace_back(relative_to_absolute(anchor,
    //      train.first_timestamp()),
    //                          relative_to_absolute(anchor,
    //                          times.front().arrival_), train.id_, trip_id,
    //                          anchor, train.path_.front());
    //
    //      auto path_idx = 1U;
    //      for (auto const [from_time, to_time] : utl::pairwise(times)) {
    //        result.emplace_back(relative_to_absolute(anchor,
    //        from_time.departure_),
    //                            relative_to_absolute(anchor,
    //                            to_time.arrival_), train.id_, trip_id, anchor,
    //                            train.path_[path_idx]);
    //        ++path_idx;
    //      }
    //
    //      // add last ms -> last halt
    //      result.emplace_back(relative_to_absolute(anchor,
    //      times.back().arrival_),
    //                          relative_to_absolute(anchor,
    //                          train.last_timestamp()), train.id_, trip_id,
    //                          anchor, train.path_.back());
    //
    //      ++trip_id;
    //
    //      utls::sassert(path_idx == train.path_.size() - 1);
    //    }

    utls::ensure(!result.empty());
    return result;
  };

  // combine route orderings must be done sequentially
  // combine_execution_policy::SEQUENTIAL
  auto const combine_route_usages = [&](auto&& end_result, auto&& partial) {
    if (partial.empty()) {
      return;
    }

    train::trip::id trip_id{
        end_result.empty() ? 0 : end_result.back().trip_id_.v_ + 1};

    for (auto [u1, u2] : utl::pairwise(partial)) {
      if (u1.trip_id_ == u2.trip_id_) {
        u1.trip_id_ = trip_id;
      } else {
        u1.trip_id_ = trip_id;
        ++trip_id;
      }
    }
    partial.back().trip_id_ = trip_id;

    utl::concat(end_result, partial);
  };

  auto result = utls::parallel_for<soro::vector<route_usage>>(
      tt->trains_, generate_route_usages, combine_route_usages);

  utls::parallel_for(result.size(), [&result](auto&& job_id) {
    result[job_id].id_ = graph::node::id{job_id};
  });

  utls::ensures([&] {
    // ensure consecutive trip ids
    for (auto const [u1, u2] : utl::pairwise(result)) {
      utls::ensure(u1.trip_id_ == u2.trip_id_ ||
                   u1.trip_id_ + 1 == u2.trip_id_);
    }
  });

  return result;
}

graph::edges_t vecs_to_vecvec(
    soro::vector<soro::vector<graph::node::id>> const& outgoing_edges) {
  utl::scoped_timer const timer("transforming vecs to vecvec");

  graph::edges_t result;

  for (auto const& edges : outgoing_edges) {
    result.emplace_back(edges);
  }

  return result;
}

graph::edges_t get_outgoing_edges(graph const& og,
                                  route_orderings const& orderings) {
  utl::scoped_timer const timer("creating outgoing edges");

  soro::vector<soro::vector<graph::node::id>> outgoing(og.nodes_.size());

  // create edges according to the sorted orderings
  {
    utl::scoped_timer const order_edge_timer("creating order edges");
    for (auto const& usage_order : orderings) {
      for (auto [from, to] : utl::pairwise(usage_order)) {
        // if the .from timestamps for the orderings are equal then we are just
        // betting that we don't introduce a cycle into the ordering graph
        //        utls::sassert(from.from_ != to.from_, "from and to are
        //        equal");
        if (from.from_ == to.from_) {
          uLOG(utl::warn) << "same from for " << from.train_id_ << " "
                          << to.train_id_ << '\n';
        }

        outgoing[from.id_.v_].emplace_back(to.id_);
      }
    }
  }

  {
    utl::scoped_timer const erase_timer("erasing duplicate edges");
    utl::parallel_for(outgoing, [](auto&& es) { utl::erase_duplicates(es); });
  }

  {
    utl::scoped_timer const train_edge_timer("creating train edges");
    for (auto const& node : og.nodes_) {
      if (node.has_next(og)) {
        outgoing[node.get_id(og).v_].emplace_back(node.next_id(og));
      }
    }
  }

  remove_transitive_dependency_edges(outgoing, og);

  return vecs_to_vecvec(outgoing);
}

graph::edges_t get_incoming_edges(graph::edges_t const& outgoing_edges) {
  utl::scoped_timer const timer("creating incoming edges");

  // 1. count how many outgoing edges every node has
  std::vector<soro::size_t> outgoing_edges_count(outgoing_edges.size());
  for (auto const edges : outgoing_edges) {
    for (auto const to : edges) {
      ++outgoing_edges_count[to.v_];
    }
  }

  // 2. depending on the outgoing edges count allocate the memory in the result
  graph::edges_t result;
  result.data_.reserve(outgoing_edges.data_.size());
  result.bucket_starts_.reserve(outgoing_edges.bucket_starts_.size());
  for (auto out_count : outgoing_edges_count) {
    // TODO(julian) stack vector would be nice here ...
    result.emplace_back(
        std::vector<graph::node::id>(out_count, graph::node::invalid()));
  }

  // 3. fill the result with the outgoing edges read from the incoming edges
  // use current_fill as a helper, to notify the bucket fill count, as we
  // allocated them all in step 2
  std::vector<soro::size_t> current_fill(outgoing_edges.size(), 0);
  for (graph::node::id to{0}; to < outgoing_edges.size(); ++to) {
    for (auto const from : outgoing_edges[to]) {
      result[from][current_fill[from.v_]] = to;
      ++current_fill[from.v_];
    }
  }

  // bonus. make sure it's correct
  utls::ensures([&] {
    utls::ensure(result.size() == outgoing_edges.size());

    // result must contain every edge from sg.train_dependencies_
    for (graph::node::id to{0}; to < outgoing_edges.size(); ++to) {
      for (auto const from : outgoing_edges[to]) {
        utls::ensure(utls::contains(result[from], to));
      }
    }
  });

  return result;
}

graph::graph(infra::infrastructure const& infra, tt::timetable const& tt)
    : graph(infra, tt, filter{}) {}

graph::graph(infra::infrastructure const& infra, tt::timetable const& tt,
             filter const& filter) {
  utl::scoped_timer const timer("creating ordering graph");

  auto const route_usages = get_route_usages(filter, tt, infra);

  auto nodes_result = get_nodes(route_usages);
  nodes_ = std::move(nodes_result.nodes_);
  trips_ = std::move(nodes_result.trips_);

  auto const orderings = get_route_orderings(route_usages, infra);

  outgoing_edges_ = get_outgoing_edges(*this, orderings);
  incoming_edges_ = get_incoming_edges(outgoing_edges_);

  print_ordering_graph_stats(*this);
}

graph::node::id graph::node::get_id(graph const& og) const {
  utls::expect(!og.nodes_.empty(), "ordering graph has no nodes");
  utls::expect(this >= og.nodes_.data(), "node is not in ordering graph");
  utls::expect(this <= og.nodes_.data() + og.nodes_.size() - 1);

  return id{utls::narrow<id::value_t>(this - og.nodes_.data())};
}

bool graph::node::has_prev(graph const& og) const {
  return !is_first_in_trip(og);
}

graph::node::id graph::node::prev_id(graph const& og) const {
  utls::sassert(has_prev(og), "node {} has no previous node", get_id(og));
  return get_id(og) - 1;
}

graph::node const& graph::node::prev(graph const& og) const {
  utls::sassert(has_prev(og), "node {} has no previous node", get_id(og));
  return og.nodes_[prev_id(og)];
}

bool graph::node::has_next(graph const& og) const {
  return !is_last_in_trip(og);
}

graph::node::id graph::node::next_id(graph const& og) const {
  utls::sassert(has_next(og), "node {} has no next node", get_id(og));
  return get_id(og) + 1;
}

graph::node const& graph::node::next(graph const& og) const {
  utls::sassert(has_next(og), "node {} has no next node", get_id(og));
  return og.nodes_[next_id(og)];
}

graph::edges_t::const_bucket graph::node::in(graph const& og) const {
  return og.incoming_edges_[get_id(og)];
}

graph::edges_t::const_bucket graph::node::out(graph const& og) const {
  return og.outgoing_edges_[get_id(og)];
}

bool graph::filter::filtered(tt::train const& train) const {
  if (include_trains_ && !utls::contains(*include_trains_, train.id_))
      [[unlikely]] {
    return true;
  }

  if (exclude_trains_ && utls::contains(*exclude_trains_, train.id_))
      [[unlikely]] {
    return true;
  }

  return false;
}

std::span<graph::node const> graph::trip_group::nodes(graph const& og) const {
  return {std::begin(og.nodes_) + from_.v_, std::begin(og.nodes_) + to_.v_};
}

bool graph::trip_group::ok() const {
  return from_ != graph::node::invalid() && to_ != graph::node::invalid() &&
         this->train_id_ != train::invalid() &&
         this->anchor_ != absolute_time::max();
}

bool graph::node::is_first_in_trip(graph const& og) const {
  return get_trip_group(og).from_ == get_id(og);
}

bool graph::node::is_last_in_trip(graph const& og) const {
  return get_trip_group(og).to_ - 1 == get_id(og);
}

graph::trip_group graph::node::get_trip_group(graph const& og) const {
  return og.trips_[this->trip_id_];
}

train::id graph::node::get_train_id(graph const& og) const {
  return og.trips_[this->trip_id_].train_id_;
}

std::string graph::node::print(graph const& og) const {
  return fmt::format("{} [train: {} ir: {}]", get_id(og),  // NOLINT
                     get_train_id(og), ir_id_);
}

graph::edges_t::const_bucket graph::out(node::id const id) const {
  return outgoing_edges_[id];
}

graph::edges_t::const_bucket graph::in(node::id const id) const {
  return incoming_edges_[id];
}

}  // namespace soro::ordering
