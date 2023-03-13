#include "soro/simulation/ordering_graph.h"

#include <random>

#include "utl/enumerate.h"
#include "utl/erase_duplicates.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/utls/container/priority_queue.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

#include "soro/runtime/runtime.h"

namespace soro::simulation {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;

struct route_usage {
  soro::absolute_time from_{};
  soro::absolute_time to_{};
  ordering_node::id id_{ordering_node::INVALID};
};

ordering_graph::ordering_graph(infra::infrastructure const& infra,
                               tt::timetable const& tt)
    : ordering_graph(infra, tt, interval{}) {}

ordering_graph::ordering_graph(infra::infrastructure const& infra,
                               tt::timetable const& tt,
                               tt::interval const& interval) {
  utl::scoped_timer const timer("creating ordering graph");

  ordering_node::id glob_current_node_id = 0;

  std::vector<std::vector<route_usage>> orderings(
      infra->exclusion_.exclusion_sets_.size());

  auto const insert_into_orderings = [&](route_usage const& usage,
                                         interlocking_route::id const ir_id) {
    for (auto const es_id : infra->exclusion_.irs_to_exclusion_sets_[ir_id]) {
      orderings[es_id].push_back(usage);
    }
  };

  auto const generate_route_orderings = [&](train const& train) {
    soro::vector<timestamp> times;

    for (auto const anchor : train.departures(interval)) {
      if (times.empty()) {
        times = runtime_calculation(train, infra, {type::MAIN_SIGNAL}).times_;

        utls::sasserts([&]() {
          auto const ms_count = utls::count_if(times, [](auto&& t) {
            return t.element_->is(type::MAIN_SIGNAL);
          });

          utls::sassert(
              train.path_.size() == ms_count + 1,
              "Differing amounts of interlocking routes in train path and "
              "main signals in running time calculation timestamps");
        });
      }

      if (times.empty()) {
        uLOG(utl::warn) << "no main signal in path of train " << train.id_;
        return;
      }

      ordering_node::id curr_node_id = ordering_node::INVALID;
      {
        curr_node_id = glob_current_node_id;
        glob_current_node_id += train.path_.size();
        nodes_.resize(nodes_.size() + train.path_.size());
      }

      {  // add first halt -> first ms
        nodes_[curr_node_id] = ordering_node{.id_ = curr_node_id,
                                             .ir_id_ = train.path_.front(),
                                             .train_id_ = train.id_,
                                             .in_ = {},
                                             .out_ = {curr_node_id + 1}};

        route_usage const first_usage = {
            .from_ = relative_to_absolute(anchor, train.first_departure()),
            .to_ = relative_to_absolute(anchor, times.front().arrival_),
            .id_ = curr_node_id};

        insert_into_orderings(first_usage, nodes_[curr_node_id].ir_id_);

        ++curr_node_id;
      }

      auto path_idx = 1U;
      for (auto const [from_time, to_time] : utl::pairwise(times)) {
        nodes_[curr_node_id] = ordering_node{.id_ = curr_node_id,
                                             .ir_id_ = train.path_[path_idx],
                                             .train_id_ = train.id_,
                                             .in_ = {curr_node_id - 1},
                                             .out_ = {curr_node_id + 1}};

        route_usage const usage = {
            .from_ = relative_to_absolute(anchor, from_time.arrival_),
            .to_ = relative_to_absolute(anchor, from_time.departure_),
            .id_ = curr_node_id};

        insert_into_orderings(usage, nodes_[curr_node_id].ir_id_);

        ++path_idx;
        ++curr_node_id;
      }

      {  // add last ms -> last halt
        nodes_[curr_node_id] = ordering_node{.id_ = curr_node_id,
                                             .ir_id_ = train.path_.back(),
                                             .train_id_ = train.id_,
                                             .in_ = {curr_node_id - 1},
                                             .out_ = {}};

        route_usage const last_usage = {
            .from_ = relative_to_absolute(anchor, times.back().arrival_),
            .to_ = relative_to_absolute(anchor, train.last_arrival()),
            .id_ = curr_node_id};

        insert_into_orderings(last_usage, nodes_[curr_node_id].ir_id_);

        ++curr_node_id;
      }

      utls::sassert(path_idx == train.path_.size() - 1);
    }
  };

  // generate the nodes and route usage orderings (1 node == 1 usage)
  for (auto const& train : tt->trains_) {
    generate_route_orderings(train);
  }

  utl::parallel_for(orderings, [](auto&& usage_order) {
    utls::sort(usage_order, [](auto&& usage1, auto&& usage2) {
      return usage1.from_ < usage2.from_;
    });
  });

  // create edges according to the sorted orderings
  for (auto const& usage_order : orderings) {
    for (auto [from, to] : utl::pairwise(usage_order)) {
      nodes_[from.id_].out_.emplace_back(to.id_);
      nodes_[to.id_].in_.emplace_back(from.id_);
    }
  }

  std::size_t edge_count = 0;
  std::size_t max_edge_per_node = 0;
  for (auto& node : nodes_) {
    utl::erase_duplicates(node.out_);
    utl::erase_duplicates(node.in_);

    edge_count += node.out_.size();
    max_edge_per_node = std::max(node.out_.size(), max_edge_per_node);
  }

  uLOG(utl::info) << "ordering graph node count: " << nodes_.size();
  uLOG(utl::info) << "ordering graph edge count: " << edge_count;
  uLOG(utl::info) << "ordering graph max edge per node: " << max_edge_per_node;
}

}  // namespace soro::simulation