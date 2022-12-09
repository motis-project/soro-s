#pragma once

#include "pugixml.hpp"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"

namespace soro::infra {

element* parse_track_element(pugi::xml_node const& track_node, type const type,
                             bool const rising, line_id const line,
                             graph& network, station& station,
                             construction_materials& mats);

speed_limit get_speed_limit(pugi::xml_node const& speed_limit_xml,
                            element::ptr element);

}  // namespace soro::infra