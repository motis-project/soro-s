#pragma once

#include <map>

#include "utl/zip.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/runtime/runtime.h"
#include "soro/simulation/dpd.h"
#include "soro/timetable/timetable.h"

namespace soro::simulation {

struct route_graph;
struct route_node;

using route_node_id = size_t;

constexpr route_node_id INVALID_ROUTE_NODE_ID =
    std::numeric_limits<route_node_id>::max();
inline constexpr bool valid(route_node_id const id) {
  return id != INVALID_ROUTE_NODE_ID;
}

struct route_node {
  soro::vector<infra::node::ptr> nodes_;
  soro::vector<infra::node::idx> omitted_nodes_;
  soro::vector<infra::speed_limit> extra_speed_limits_;

  route_node() = delete;
  route_node(route_node_id const id, tt::train::ptr train)
      : id_(id), name_(soro::string(std::to_string(id))), train_(train) {}

  [[nodiscard]] auto const& same_train_dep() const { return in_.front(); }
  auto& same_train_dep() { return in_.front(); }

  [[nodiscard]] auto const& same_train_succ() const { return out_.front(); }
  auto& same_train_succ() { return out_.front(); }

  [[nodiscard]] bool is_partial() const { return main_ != id_; }
  [[nodiscard]] bool is_init() const { return nodes_.empty(); }

  [[nodiscard]] bool finished() const {
    return (is_init() && !exit_dpd_.empty()) || (!exit_dpd_.empty());
  }

  void compute_dists();

  std::vector<infra::station::ptr> participating_stations_;

  route_node_id id_{INVALID_ROUTE_NODE_ID};
  soro::string name_;

  // TODO(julian) i dont like this pointer here, maybe change to id?
  tt::train::ptr train_{nullptr};
  bool halt_{false};

  route_node_id main_{INVALID_ROUTE_NODE_ID};
  std::vector<route_node_id> in_, out_;

  utls::unixtime timetable_dep_{utls::INVALID_TIME};

  dpd<default_granularity, utls::unixtime, kilometer_per_hour> entry_dpd_{};
  dpd<default_granularity, utls::unixtime, kilometer_per_hour> exit_dpd_{};
  dpd<default_granularity, utls::unixtime> eotd_dpd_{};
};

struct occupancy {
  route_node_id route_node_id_{INVALID_ROUTE_NODE_ID};
  tt::train::id train_id_{tt::train::INVALID};
  utls::unixtime from_{0};
  bool operator==(occupancy const& other) const {
    return (from_ == other.from_ && route_node_id_ == other.route_node_id_);
  }
};

struct route_graph {
  route_node const& get(route_node_id const id) const { return nodes_[id]; }

  route_node_id create_new_route_node(tt::train const* train) {
    route_node_id const id = nodes_.size();
    nodes_.emplace_back(id, train);

    auto it = train_name_to_nodes_.find(train->name_);
    if (it != std::end(train_name_to_nodes_)) {
      it->second.push_back(id);
    } else {
      train_name_to_nodes_[train->name_] = {id};
    }

    return id;
  }

  std::vector<route_node> nodes_;
  std::map<soro::string, std::vector<route_node_id>> train_name_to_nodes_;
  std::vector<std::vector<route_node_id>> conflicts_;

  std::vector<runtime::timestamps> runtime_timestamps_;
  std::map<infra::element::ptr, std::vector<occupancy>> element_occupancies_;
};

route_graph get_route_graph(infra::infrastructure_t const& iss,
                            tt::timetable const& dt);

inline bool ready(route_graph const& rg, route_node const& node) {
  return std::all_of(begin(node.in_), end(node.in_), [&](auto in) {
    return in == INVALID_ROUTE_NODE_ID || rg.get(in).finished();
  });
}

soro::vector<infra::interlocking_route> get_signal_station_routes(
    tt::train const& tr);

infra::interlocking_subsystem get_ssr_manager(tt::timetable const& dt);

}  // namespace soro::simulation
