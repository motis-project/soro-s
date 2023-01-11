#pragma once

#include "soro/infrastructure/infra_stats.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/iss_types.h"

namespace soro::infra {

struct omitted_rp_node {
  bool operator==(omitted_rp_node const& o) const {
    return this->rp_node_id_ == o.rp_node_id_ && this->type_ == o.type_;
  }

  rail_plan_node_id rp_node_id_;
  type type_;
};

struct intermediate_station_route {
  rail_plan_node_id start_{INVALID_RP_NODE_ID};
  rail_plan_node_id end_{INVALID_RP_NODE_ID};
  rail_plan_node_id rp_passenger_halt_{INVALID_RP_NODE_ID};
  rail_plan_node_id rp_freight_halt_{INVALID_RP_NODE_ID};
  rail_plan_node_id rp_runtime_checkpoint_{INVALID_RP_NODE_ID};

  soro::vector<omitted_rp_node> omitted_rp_nodes_;

  soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()> attributes_{
      DEFAULT_ATTRIBUTE_ARRAY};

  soro::vector<course_decision> course_;

  soro::vector<speed_limit> extra_speed_limits_;

  station_route::id id_;
  soro::string name_;
  soro::ptr<station> station_;
};

struct construction_materials {
  std::size_t section_counts_{};
  infra_stats total_element_counts_{};
  soro::map<rail_plan_node_id, element_id> rp_id_to_element_id_{};

  soro::vector<intermediate_station_route> intermediate_station_routes_;
};

}  // namespace soro::infra