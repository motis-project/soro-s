#pragma once

#include "soro/infrastructure/infra_stats.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/iss_types.h"

namespace soro::infra {

struct intermediate_station_route {
  rail_plan_node_id start_{INVALID_RP_NODE_ID};
  rail_plan_node_id end_{INVALID_RP_NODE_ID};
  rail_plan_node_id rp_passenger_halt_{INVALID_RP_NODE_ID};
  rail_plan_node_id rp_freight_halt_{INVALID_RP_NODE_ID};

  soro::vector<rail_plan_node_id> omitted_rp_nodes_;

  soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()> attributes_{
      DEFAULT_ATTRIBUTE_ARRAY};

  soro::vector<course_decision> course_;
  soro::vector<speed_limit> extra_speed_limits_;

  std::size_t id_;
  soro::string name_;
  soro::ptr<station> station_;
};

struct construction_materials {
  std::size_t section_counts_{};
  infra_stats total_element_counts_{};
  soro::map<rail_plan_node_id, element_id> rp_id_to_element_id_{};

  std::vector<intermediate_station_route> intermediate_station_routes_;
};

}  // namespace soro::infra