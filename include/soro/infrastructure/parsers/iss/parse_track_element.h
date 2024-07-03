#pragma once

#include "pugixml.hpp"

#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"

namespace soro::infra {

element* parse_track_element(pugi::xml_node const& track_node,
                             mileage_dir const dir, line::id const line,
                             dictionaries const& dicts, graph& network,
                             station& station, construction_materials& mats);

speed_limit get_speed_limit(pugi::xml_node const& speed_limit_xml,
                            dictionaries const& dicts,
                            speed_limit::source const source);

}  // namespace soro::infra