#pragma once

#include "soro/infrastructure/infra_stats.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using rail_plan_node_id = uint64_t;

constexpr auto const INVALID_RP_NODE_ID =
    std::numeric_limits<rail_plan_node_id>::max();

struct intermediate_station_route {
  struct speed_limit {
    speed_limit(infra::speed_limit const& other, rail_plan_node_id const rp_id)
        : spl_{other}, rp_id_{rp_id} {}

    infra::speed_limit spl_;
    rail_plan_node_id rp_id_{INVALID_RP_NODE_ID};
  };

  struct omitted {
    soro::vector<rail_plan_node_id> nodes_;
    soro::vector<type> types_;
  };

  struct alternative_speed_limit {
    rail_plan_node_id node_id_;
    si::speed speed_;
  };

  rail_plan_node_id start_{INVALID_RP_NODE_ID};
  rail_plan_node_id end_{INVALID_RP_NODE_ID};

  soro::optional<rail_plan_node_id> passenger_halt_;
  soro::optional<rail_plan_node_id> freight_halt_;
  soro::optional<rail_plan_node_id> runtime_checkpoint_;

  omitted omitted_;

  soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()> attributes_{
      DEFAULT_ATTRIBUTE_ARRAY};

  soro::vector<course_decision> course_;

  soro::vector<speed_limit> speed_limits_;
  soro::vector<alternative_speed_limit> alt_speed_limits_;

  station_route::id id_;
  soro::string name_;
  soro::ptr<station> station_;
};

struct id_mapping {
  element::id operator[](rail_plan_node_id const id,
                         mileage_dir const dir) const {
    utls::expect(id < mapping_.size());
    return mapping_[id][std::to_underlying(dir)];
  }

  std::array<element::id, 2> operator[](rail_plan_node_id const id) const {
    utls::expect(id < mapping_.size());
    return mapping_[id];
  }

  soro::size_t size() const {
    return utls::narrow<soro::size_t>(mapping_.size());
  }

  element::id first(rail_plan_node_id const id) const {
    utls::expect(id < mapping_.size());
    return mapping_[id][0];
  }

  element::id second(rail_plan_node_id const id) const {
    utls::expect(id < mapping_.size());
    return mapping_[id][1];
  }

  void add(rail_plan_node_id const rp_id, element::id const e_id) {
    if (mapping_.size() <= rp_id) {
      utls::expect(rp_id < std::numeric_limits<rail_plan_node_id>::max());
      mapping_.resize(rp_id + 1, {element::invalid(), element::invalid()});
    }

    if (mapping_[rp_id][0] == element::invalid()) {
      mapping_[rp_id][0] = e_id;
      return;
    }

    utls::sassert(mapping_[rp_id][1] == element::invalid(), "no overwriting");
    mapping_[rp_id][1] = e_id;
  }

  std::vector<std::array<element::id, 2>> mapping_;
};

struct construction_materials {
  std::size_t section_counts_;

  infra_stats total_element_counts_;

  id_mapping to_element_id_;

  soro::vector<intermediate_station_route> intermediate_station_routes_;
};

}  // namespace soro::infra