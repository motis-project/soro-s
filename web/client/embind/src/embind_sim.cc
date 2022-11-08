
#include "emscripten/bind.h"

#include "soro/simulation/granularity.h"
#include "soro/simulation/sim_graph.h"

#include "soro/web/register_pair.h"
#include "soro/web/register_vector.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;
using namespace soro::simulation;

EMSCRIPTEN_BINDINGS(unixtime) {
  emscripten::class_<utls::unixtime>("Unixtime")
      .property("t", &utls::unixtime::t_);
}

EMSCRIPTEN_BINDINGS(kmh) {
  emscripten::class_<kilometer_per_hour>("KM/H").property(
      "km_h", &kilometer_per_hour::km_h_);
}

auto get_time_speed_granularity(TimeSpeedDPD const&) {
  return std::pair{TimeSpeedDPD::get_granularity<utls::unixtime>(),
                   TimeSpeedDPD::get_granularity<kilometer_per_hour>()};
}

EMSCRIPTEN_BINDINGS(dpd) {
  register_vector<probability_t>("ProbablityList");
  register_vector<dpd<default_granularity, kilometer_per_hour>>("DPDKMHLIST");

  register_pair<utls::unixtime, kilometer_per_hour>("TimeSpeedPair");

  emscripten::class_<TimeDPD>("TimeDPD")
      .property("first", &TimeDPD ::first_)
      .property("dpd", &TimeDPD::dpd_);

  emscripten::class_<TimeSpeedDPD>("TimeSpeedDPD")
      .function("get_granularity", &get_time_speed_granularity)
      .property("first", &TimeSpeedDPD ::first_)
      .property("dpd", &TimeSpeedDPD::dpd_);
}

EMSCRIPTEN_BINDINGS(sim_graph) {
  emscripten::class_<sim_node>("SimNode")
      .property("ID", &sim_node::id_)
      .property("TrainID", &sim_node::train_id_)
      .property("SSRID", &sim_node::ir_id_)
      .property("trainSucc", &sim_node::train_successor_)
      .property("trainPred", &sim_node::train_predecessor_)
      .function("hasSucc", &sim_node::has_succ)
      .function("hasPred", &sim_node::has_pred)
      .function("In", &sim_node::in)
      .function("Out", &sim_node::out)
      .function("OrderIn", &sim_node::order_in)
      .function("OrderOut", &sim_node::order_out);

  register_vector<sim_node>("SimNodeList");
  register_vector<std::pair<size_t, size_t>>("SimNodeRangeList");

  register_pair<size_t, size_t>("SizeTPair");

  emscripten::class_<sim_graph>("SimGraph")
      .constructor<infra::infrastructure const&, tt::timetable const&>()
      .property("nodes", &sim_graph::nodes_)
      .property("train_to_sim_nodes", &sim_graph::train_to_sim_nodes_);

  emscripten::function("simulate", &simulate);
}