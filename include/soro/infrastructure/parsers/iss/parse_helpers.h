#pragma once

#include "pugixml.hpp"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"

namespace soro::infra {

kilometrage parse_kilometrage(std::string_view const kmp);

rail_plan_node_id parse_rp_node_id(pugi::xml_node const& node);

rail_plan_node_id parse_rp_node_id_attribute(pugi::xml_node const& node);

bool has_rising_name(pugi::xml_node const& node);

mileage_dir get_track_element_direction(pugi::xml_node const& node);

si::speed parse_speed(pugi::xml_node const& speed_xml);

si::length parse_length(pugi::xml_node const& length_xml);

}  // namespace soro::infra
