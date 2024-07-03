#include "soro/infrastructure/parsers/iss/parse_station_route.h"

#include "utl/logging.h"

#include "soro/base/soro_types.h"

#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/parse_helpers.h"
#include "soro/infrastructure/parsers/iss/parse_track_element.h"
#include "soro/infrastructure/station/station_route.h"

namespace soro::infra {

using namespace pugi;

using namespace utl;

soro::vector<intermediate_station_route::alternative_speed_limit>
parse_alternative_values(xml_node const& alternative_values) {
  soro::vector<intermediate_station_route::alternative_speed_limit> result;

  for (auto const& alternative_value : alternative_values.children()) {
    intermediate_station_route::alternative_speed_limit aspl;

    auto const node_xml = alternative_value.child(NODES);
    auto const speed_xml = alternative_value.child(SPEED);

    // TODO(julian) only alternative values for speed limits are supported
    // there are very few instances of alternative values for brake path
    // which *should* be supported in the future
    if (!static_cast<bool>(speed_xml)) {
      continue;
    }

    utls::sassert(
        get_type(node_xml.attribute(TYPE).value()) == type::SPEED_LIMIT,
        "only speed limits can have alternate speed values");

    aspl.speed_ = parse_speed(speed_xml);
    aspl.node_id_ = parse_rp_node_id_attribute(node_xml);

    result.push_back(aspl);
  }

  return result;
}

intermediate_station_route parse_station_route(
    station_route::id const id, pugi::xml_node const& xml_station_route,
    soro::ptr<station> station) {
  intermediate_station_route result;

  result.id_ = id;

  result.name_ = xml_station_route.child_value(NAME);
  result.station_ = station;
  result.start_ = utls::parse_int<rail_plan_node_id>(
      xml_station_route.child(BEGIN).attribute(ID).value());
  result.end_ = utls::parse_int<rail_plan_node_id>(
      xml_station_route.child(END).attribute(ID).value());

  if (auto const rtc = xml_station_route.child(RUNTIME_CHECKPOINT); rtc) {
    result.runtime_checkpoint_.emplace(parse_rp_node_id_attribute(rtc));
  }

  if (auto const passenger_halt = xml_station_route.child(PASSENGER_STOP);
      passenger_halt) {
    result.passenger_halt_.emplace(parse_rp_node_id_attribute(passenger_halt));
  }

  if (auto const freight_halt = xml_station_route.child(FREIGHT_STOP);
      freight_halt) {
    result.freight_halt_.emplace(parse_rp_node_id_attribute(freight_halt));
  }

  if (auto const omitted_nodes = xml_station_route.child(OMITTED_NODES);
      omitted_nodes) {
    for (auto const& omit_node : omitted_nodes.children(OMITTED_NODE)) {
      result.omitted_.nodes_.emplace_back(
          parse_rp_node_id_attribute(omit_node));
      result.omitted_.types_.emplace_back(
          get_type(omit_node.attribute(TYPE).value()));
    }
  }

  for (auto const& at : xml_station_route.child(ATTRIBUTES).children()) {
    result.attributes_[attribute_index(at.name())] = true;
  }

  for (auto const& cd : xml_station_route.child(COURSE).children()) {
    if (utls::equal(cd.name(), COURSE_STEM)) {
      result.course_.push_back(course_decision::STEM);
    } else {
      result.course_.push_back(course_decision::BRANCH);
    }
  }

  for (auto const& xml_spl : xml_station_route.child(SPEED_LIMITS).children()) {
    if (utls::equal(xml_spl.child(NODES).attribute(TYPE).value(),
                    WARNING_SIGN)) {
      uLOG(warn) << "skipped one station route speed limit with node type "
                 << WARNING_SIGN << " in station route " << result.name_
                 << " in station " << station->ds100_ << ".";
      continue;
    }

    auto const rp_id = parse_rp_node_id_attribute(xml_spl.child(NODES));
    result.speed_limits_.emplace_back(
        get_speed_limit(xml_spl, {}, speed_limit::source::station_route),
        rp_id);
  }

  result.alt_speed_limits_ =
      parse_alternative_values(xml_station_route.child(ALTERNATIVE_VALUES));

  utls::ensure(result.omitted_.nodes_.size() == result.omitted_.types_.size());

  return result;
}

}  // namespace soro::infra
