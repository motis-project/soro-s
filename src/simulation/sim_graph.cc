#include "soro/simulation/sim_graph.h"

#include <set>

#include "soro/utls/graph/traversal.h"
#include "soro/utls/sassert.h"
#include "soro/utls/unixtime.h"

#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/runtime/runtime.h"
#include "soro/simulation/disruption.h"
#include "soro/simulation/ordering_graph.h"

namespace soro::simulation {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;
using namespace soro::utls::literals;

bool sim_node::finished(simulation_result const& sr) const {
  return !sr[this->id_].exit_dpd_.empty();
}

bool sim_node::ready(sim_graph const& sg, simulation_result const& sr) const {
  return utls::all_of(this->in(),
                      [&](auto&& in) { return sg.nodes_[in].finished(sr); });
}

bool sim_node::has_succ() const { return train_succ() != sim_node::INVALID; }
bool sim_node::has_pred() const { return train_pred() != sim_node::INVALID; }

sim_node::id sim_node::train_succ() const { return train_successor_; }
sim_node::id sim_node::train_pred() const { return train_predecessor_; }

std::vector<sim_node::id> const& sim_node::order_in() const { return in_; }
std::vector<sim_node::id> const& sim_node::order_out() const { return out_; }

utls::generator<const sim_node::id> sim_node::out() const {
  if (has_succ()) {
    co_yield train_succ();
  }

  for (auto const& out : order_out()) {
    co_yield out;
  }
}

utls::generator<const sim_node::id> sim_node::in() const {
  if (has_pred()) {
    co_yield train_pred();
  }

  for (auto const& in : order_in()) {
    co_yield in;
  }
}

sim_graph::sim_graph(infra::infrastructure const& infra,
                     tt::timetable const& tt)
    : infra_{infra}, timetable_{tt} {

  // --- Create the simulation nodes --- //
  train_to_sim_nodes_.resize(tt->trains_.size());
  ssr_to_node_.resize(tt->trains_.size());

  for (auto const& tr : tt->trains_) {
    auto const first_node = static_cast<sim_node::id>(nodes_.size());

    sim_node init_sn;
    init_sn.id_ = static_cast<sim_node::id>(nodes_.size());
    init_sn.train_id_ = tr.id_;
    init_sn.ir_id_ = interlocking_route::INVALID;
    //    init_sn.exit_dpd_ = get_init_dpd(tr->first_departure());
    nodes_.push_back(init_sn);

    utls::sassert(false, "Not implemented");
    //    for (auto const& ir_id : tr->path_) {
    //      sim_node sn;
    //      sn.id_ = nodes_.size();
    //      sn.train_id_ = tr->id_;
    //      sn.ir_id_ = ir_id;
    //
    //      nodes_.push_back(sn);
    //
    //      ssr_to_node_[tr->id_][ir_id] = sn.id_;
    //    }

    // create two sentinel node at the end of the simulation train run
    nodes_.push_back({
        .id_ = static_cast<sim_node::id>(nodes_.size()),
        .train_id_ = tr.id_,
        .ir_id_ = interlocking_route::INVALID,
    });

    nodes_.push_back({
        .id_ = static_cast<sim_node::id>(nodes_.size()),
        .train_id_ = tr.id_,
        .ir_id_ = interlocking_route::INVALID,
    });

    train_to_sim_nodes_[tr.id_] = {first_node,
                                   static_cast<sim_node::id>(nodes_.size())};
  }

  // --- Create simulation graph edges --- //

  // first all 'trivial' edges; connect nodes belonging to a single train run
  for (auto& sn : nodes_) {
    if (sn.id_ != train_to_sim_nodes_[sn.train_id_].first) {
      sn.train_predecessor_ = sn.id_ - 1;
    }

    if (sn.id_ != train_to_sim_nodes_[sn.train_id_].second - 1) {
      sn.train_successor_ = sn.id_ + 1;
    }
  }

  //  ordering_graph const og(infra, tt);

  // these are the ordering edges; these impose an order in which trains
  // use shared/contested resources (signal station routes)
  //  auto const ordering = get_route_ordering(infra, tt);
  //
  //  for (auto const& train_run : tt) {
  //    for (auto const& ssr : train_run->ssr_run_.path_) {
  //      for (auto const [first, second] : utl::pairwise(ordering[ssr->id_])) {
  //        auto first_sim_node_id = ssr_to_node_[first.train_id_].at(ssr->id_);
  //        auto second_sim_node_id =
  //        ssr_to_node_[second.train_id_].at(ssr->id_);
  //
  //        // create the actual order edge
  //        auto const from = first_sim_node_id + 2;
  //        auto const to = second_sim_node_id;
  //
  //        nodes_[from].out_.push_back(to);
  //        nodes_[to].in_.push_back(from);
  //      }
  //    }
  //  }
  //
  //  // TODO(julian) instead of erasing, do not add them in the first place
  //  for (auto& node : nodes_) {
  //    utls::unique_erase(node.in_);
  //    utls::unique_erase(node.out_);
  //  }

  //  utl::verify(!cycle_detection(*this), "Found cycle in sim_graph!");
}

bool sim_graph::has_cycle() const {
  std::vector<bool> visited(nodes_.size());
  std::vector<bool> finished(nodes_.size());

  auto const cycle_detection = [&](sim_node::id const n_outer) {
    auto const cycle_detection_impl = [&](sim_node::id const n,
                                          auto&& impl) -> bool {
      if (finished[n]) {
        return false;
      }

      if (visited[n]) {
        return true;
      }

      visited[n] = true;

      for (auto const& neighbour : nodes_[n].out()) {
        if (impl(neighbour, impl)) {
          return true;
        }
      }

      finished[n] = true;

      return false;
    };

    return cycle_detection_impl(n_outer, cycle_detection_impl);
  };

  return utls::any_of(train_to_sim_nodes_,
                      [&](auto&& pair) { return cycle_detection(pair.first); });
}

bool sim_graph::path_exists(sim_node::id const from,
                            sim_node::id const to) const {
  bool result = false;

  auto const handle_node = [&](sim_node::id const node, sim_node::id const) {
    result = node == to;
    return result;
  };

  auto const get_neighbours = [&](sim_node::id const node) {
    return nodes_[node].out();
  };

  utls::bfs(from, handle_node, get_neighbours);

  return result;
}

sim_node_result const& simulation_result::operator[](
    uint32_t const idx) const noexcept {
  return results_[idx];
}

TimeDPD fold_max(TimeDPD const& dpd1, TimeDPD const& dpd2) {
  TimeDPD result;

  for (auto const [t1, p1] : dpd1) {
    for (auto const [t2, p2] : dpd2) {
      result.insert(std::max(t1, t2), p1 * p2);
    }
  }

  return result;
}

void simulation_result::compute_dists(const sim_node::id, sim_graph const&,
                                      simulation_options const&) const {
  std::ignore = this->results_;
  utls::sassert(false, "Not implemented");
}

simulation_result::simulation_result(sim_graph const&) {
  utls::sassert(false, "Not implemented");
}

simulation_result simulate(sim_graph const& sg,
                           simulation_options const& opts) {
  simulation_result result(sg);

  std::set<sim_node::id> todo;

  for (auto const& sn : sg.nodes_) {
    if (sn.finished(result)) {
      continue;
    } else if (sn.ready(sg, result)) {
      todo.emplace(sn.id_);
    }
  }

  while (!todo.empty()) {
    auto const next = *begin(todo);
    todo.erase(begin(todo));
    auto& next_sn = sg.nodes_[next];
    result.compute_dists(next, sg, opts);
    for (auto const& out : next_sn.out()) {
      if (out != sim_node::INVALID && sg.nodes_[out].ready(sg, result)) {
        todo.emplace(out);
      }
    }
  }

  return result;
}

}  // namespace soro::simulation
