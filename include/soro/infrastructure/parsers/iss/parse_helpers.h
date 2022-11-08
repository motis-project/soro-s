#pragma once

#include "pugixml.hpp"

#include "iss_types.h"
#include "soro/infrastructure/graph/element.h"

namespace soro::infra {

kilometrage parse_kilometrage(pugi::xml_node const& node);

rail_plan_node_id parse_rp_node_id(pugi::xml_node const& node);

bool has_rising_name(pugi::xml_node const& node);

}  // namespace soro::infra
