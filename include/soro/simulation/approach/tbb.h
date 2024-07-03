#pragma once

#if defined(SORO_TBB)

#include "oneapi/tbb/flow_graph.h"
#include "oneapi/tbb/global_control.h"

#include "utl/timer.h"

#include "soro/simulation/graph.h"
#include "soro/simulation/result/simulation.h"

namespace soro::simulation {

template <typename RuntimeCalculator>
auto simulate_tbb(graph const& g, RuntimeCalculator const& runtime_calculator,
                  std::size_t const threads)
    -> result<typename RuntimeCalculator::simulation_node_data> {
  using namespace oneapi::tbb;

  utl::scoped_timer const timer("tbb dependency graph approach");
  uLOG(utl::info) << "using " << threads << " threads";

  global_control const c(global_control::max_allowed_parallelism, threads);

  using result_t = result<typename RuntimeCalculator::simulation_node_data>;

  result_t result(g.nodes_.size());

  // wrapper to use the runtime calculator in the flow graph
  struct wrapped_calc {
    wrapped_calc(graph const& g, graph::simulation_group const& sg,
                 RuntimeCalculator const& runtime_calculator, result_t& result)
        : g_{g},
          sg_{sg},
          runtime_calculator_{runtime_calculator},
          result_{result} {}

    flow::continue_msg operator()(flow::continue_msg) {
      runtime_calculator_(sg_, g_, result_, result_.result_span(sg_));
      return {};
    }

    graph const& g_;  // NOLINT
    graph::simulation_group const& sg_;  // NOLINT
    RuntimeCalculator const& runtime_calculator_;  // NOLINT
    result_t& result_;  // NOLINT
  };

  flow::graph fg;

  // create flow nodes and remember nodes that can be started
  using node_t = flow::continue_node<flow::continue_msg>;

  std::vector<node_t> nodes;
  nodes.reserve(g.simulation_groups_.size());

  std::vector<uint32_t> start_nodes;
  start_nodes.reserve(g.simulation_groups_.size());

  for (auto const& sg : g.simulation_groups_) {
    nodes.emplace_back(fg, wrapped_calc(g, sg, runtime_calculator, result));

    if (!sg.has_previous(g) && !sg.has_train_dependencies(g)) {
      start_nodes.emplace_back(sg.get_id(g).v_);
    }
  }

  // create flow edges
  for (auto const& from : g.simulation_groups_) {
    if (from.has_previous(g)) {
      flow::make_edge(nodes[from.get_id(g).v_ - 1], nodes[from.get_id(g).v_]);
    }

    for (auto const& to : from.get_train_dependents(g)) {
      flow::make_edge(nodes[from.get_id(g).v_], nodes[to.v_]);
    }
  }

  // start calculation for all valid start nodes
  for (auto const start : start_nodes) {
    nodes[start].try_put(flow::continue_msg());
  }

  fg.wait_for_all();

  return result;
}

}  // namespace soro::simulation

#endif  // SORO_TBB