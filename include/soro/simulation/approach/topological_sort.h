#pragma once

#include "utl/timer.h"

#include "soro/utls/graph/traversal.h"
#include "soro/utls/std_wrapper/reverse.h"

#include "soro/simulation/simulator/simulator.h"

namespace soro::sim::topo {

soro::vector<graph::simulation_group::id> get_topological_order(
    graph const& g) {
  utl::scoped_timer const timer("generating topological order");

  soro::vector<graph::simulation_group::id> order;
  order.reserve(g.simulation_groups_.size());

  std::vector<graph::simulation_group::id> roots;
  roots.reserve(g.simulation_groups_.size());

  for (auto const& sg : g.simulation_groups_) {
    if (!sg.has_train_dependencies(g)) {
      roots.emplace_back(sg.get_id(g));
    }
  }

  std::vector<bool> permanent(g.simulation_groups_.size(), false);
  std::vector<bool> temporary(g.simulation_groups_.size(), false);

  auto const visit = [&](graph::simulation_group::id const sg_id,
                         auto&& visit_recursive) {
    if (permanent[sg_id.v_]) {
      return;
    }

    if (temporary[sg_id.v_]) {
      uLOG(utl::err) << "cycle in graph\n";
    }

    temporary[sg_id.v_] = true;

    for (auto const dep : g.simulation_groups_[sg_id].get_train_dependents(g)) {
      visit_recursive(dep, visit_recursive);
    }

    temporary[sg_id.v_] = false;
    permanent[sg_id.v_] = true;
    order.emplace_back(sg_id);
  };

  for (auto const root_id : roots) {
    visit(root_id, visit);
  }

  utls::reverse(order);

  utls::ensure(order.size() == g.simulation_groups_.size(),
               "order of sim groups has to include all groups");
  //  utls::ensure(utls::all_of(permanent));

  return order;
}

simulator::results_t simulate(infra::infrastructure const& infra,
                              tt::timetable const& tt, sim::graph const& sg) {
  utl::scoped_timer const timer("topological sort approach");

  simulator simulator(infra, tt, sg);

  auto const order = get_topological_order(sg);

  for (auto const& id : order) {
    auto const& current = sg.simulation_groups_[id];
    simulator(current, sg, tt);
  }

  return std::move(simulator.results_);
}

}  // namespace soro::sim::topo