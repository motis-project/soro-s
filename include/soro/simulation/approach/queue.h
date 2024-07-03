#pragma once

#include "utl/timer.h"

#include "soro/simulation/simulator/simulator.h"

namespace soro::sim::queue {

simulator::results_t simulate(infra::infrastructure const& infra,
                              tt::timetable const& tt, graph const& g) {
  utl::scoped_timer const timer("simulating with queue approach");

  simulator simulator(infra, tt, g);

  std::vector<bool> finished(g.simulation_groups_.size(), false);

  std::vector<graph::simulation_group::id> todo;
  todo.reserve(g.simulation_groups_.size());

  // initial fill
  for (auto const& sg : g.simulation_groups_) {
    if (!sg.has_previous(g) && !sg.has_train_dependencies(g)) {
      todo.emplace_back(sg.get_id(g));
    }
  }

  uLOG(utl::info) << "initial todo items: " << todo.size();

  auto const ready = [&](graph::simulation_group const& sg) {
    bool r = sg.has_previous(g) && finished[sg.previous(g).get_id(g).v_];

    for (auto const dep : sg.get_train_dependencies(g)) {
      r &= finished[dep.v_];
    }

    return r;
  };

  while (!todo.empty()) {
    auto const& current = g.simulation_groups_[todo.back()];
    finished[todo.back().v_] = true;
    todo.pop_back();

    simulator(current, g, tt);

    if (current.has_next(g) && ready(current.next(g))) {
      todo.emplace_back(current.next(g).get_id(g));
    }

    for (auto const dependent : current.get_train_dependents(g)) {
      if (!finished[dependent.v_] && ready(g.simulation_groups_[dependent])) {
        todo.emplace_back(dependent);
      }
    }
  }

  //  utls::ensure(utls::all_of(finished), "not all nodes processed");

  return std::move(simulator.results_);
}

}  // namespace soro::sim::queue
