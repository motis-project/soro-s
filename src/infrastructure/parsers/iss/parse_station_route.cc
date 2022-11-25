#include "soro/infrastructure/parsers/iss/parse_station_route.h"

#include "utl/logging.h"

#include "soro/utls/parse_int.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/parsers/iss/parse_track_element.h"

namespace soro::infra {

using namespace utl;

intermediate_station_route parse_station_route(
    std::size_t const id, pugi::xml_node const& xml_station_route,
    soro::ptr<station> station, graph const& network,
    construction_materials const& mats) {
  intermediate_station_route i_station_route;
  i_station_route.id_ = id;

  i_station_route.name_ = xml_station_route.child_value(NAME);
  i_station_route.station_ = station;
  i_station_route.start_ =
      std::stoul(xml_station_route.child(BEGIN).attribute(ID).value());
  i_station_route.end_ =
      std::stoul(xml_station_route.child(END).attribute(ID).value());

  auto const passenger_halt = xml_station_route.child(PASSENGER_STOP);
  if (static_cast<bool>(passenger_halt)) {
    i_station_route.rp_passenger_halt_ =
        std::stoul(passenger_halt.attribute(ID).value());
  }

  auto const freight_halt = xml_station_route.child(FREIGHT_STOP);
  if (static_cast<bool>(freight_halt)) {
    i_station_route.rp_freight_halt_ =
        std::stoul(freight_halt.attribute(ID).value());
  }

  auto const omitted_nodes = xml_station_route.child(OMITTED_NODES);
  if (static_cast<bool>(omitted_nodes)) {
    for (auto const& omit_node : omitted_nodes.children(OMITTED_NODE)) {
      std::integral auto const rp_node_id =
          utls::parse_int<rail_plan_node_id>(omit_node.attribute(ID).value());
      auto const type = get_type(omit_node.attribute(TYPE).value());
      i_station_route.omitted_rp_nodes_.push_back(
          omitted_rp_node{.rp_node_id_ = rp_node_id, .type_ = type});
    }
  }

  for (auto const& at : xml_station_route.child(ATTRIBUTES).children()) {
    i_station_route.attributes_[attribute_index(at.name())] = true;
  }

  for (auto const& cd : xml_station_route.child(COURSE).children()) {
    if (std::strcmp(cd.name(), COURSE_STEM) == 0) {
      i_station_route.course_.push_back(course_decision::STEM);
    } else {
      i_station_route.course_.push_back(course_decision::BRANCH);
    }
  }

  for (auto const& xml_spl : xml_station_route.child(SPEED_LIMITS).children()) {

    if (utls::equal(xml_spl.child(NODES).attribute(TYPE).value(),
                    WARNING_SIGN)) {
      uLOG(warn) << "Skipped one station route speed limit with node type "
                 << WARNING_SIGN << " in station route "
                 << i_station_route.name_ << " in station " << station->ds100_
                 << ".";
      continue;
    }

    auto const infra_id = mats.rp_id_to_element_id_.at(
        std::stoul(xml_spl.child(NODES).attribute(ID).value()));
    auto const spl = get_speed_limit(xml_spl, network.elements_[infra_id]);
    i_station_route.extra_speed_limits_.push_back(spl);
  }

  return i_station_route;
}

}  // namespace soro::infra
