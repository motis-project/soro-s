#pragma once

#include "utl/get_or_create.h"

#include "soro/utls/coroutine/generator.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/simulation/dpd.h"
#include "soro/simulation/granularity.h"
#include "soro/simulation/simulation_options.h"
#include "soro/timetable/timetable.h"

namespace soro::simulation {

struct sim_graph;
struct simulation_result;

struct sim_node {
  using id = uint32_t;
  static constexpr id INVALID = std::numeric_limits<id>::max();

  bool finished(simulation_result const&) const;
  bool ready(sim_graph const&, simulation_result const& sr) const;

  bool has_succ() const;
  bool has_pred() const;

  id train_succ() const;
  id train_pred() const;

  std::vector<id> const& order_in() const;
  std::vector<id> const& order_out() const;

  utls::generator<const id> out() const;
  utls::generator<const id> in() const;

  id id_{INVALID};

  tt::train::id train_id_{tt::train::INVALID};
  infra::interlocking_route::id ir_id_{infra::interlocking_route::INVALID};

  id train_predecessor_{INVALID};
  id train_successor_{INVALID};

  std::vector<id> in_{};
  std::vector<id> out_{};
};

struct sim_graph {
  sim_graph(infra::infrastructure const&, tt::timetable const&);

  bool has_cycle() const;
  bool path_exists(sim_node::id const from, sim_node::id const to) const;

  std::vector<sim_node> nodes_;
  // Pairs of indices into the nodes_ vector, giving a [from, to) range for
  // sim_nodes belonging to a given train
  std::vector<std::pair<tt::train::id, sim_node::id>> train_to_sim_nodes_;
  std::vector<std::map<infra::interlocking_route::id, sim_node::id>>
      ssr_to_node_;

  infra::infrastructure const& infra_;
  tt::timetable const& timetable_;
};

using TimeDPD = dpd<default_granularity, utls::unixtime>;
using DurationDPD = dpd<default_granularity, utls::duration>;
using TimeSpeedDPD =
    dpd<default_granularity, utls::unixtime, kilometer_per_hour>;

struct sim_node_result {
  TimeSpeedDPD entry_dpd_;
  TimeSpeedDPD exit_dpd_;
  TimeDPD eotd_dpd_;
};

struct simulation_result {
  explicit simulation_result(sim_graph const& sg);

  sim_node_result const& operator[](uint32_t const idx) const noexcept;

  void compute_dists(sim_node::id const sn_id, sim_graph const& sg,
                     simulation_options const& opts);

  soro::vector<sim_node_result> results_;
};

simulation_result simulate(sim_graph const& sg, simulation_options const& opts);

}  // namespace soro::simulation