#pragma once

#include "pugixml.hpp"

#include "soro/base/soro_types.h"

#include "soro/infrastructure/station/station.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/infrastructure/parsers/iss/construction_materials.h"

namespace soro::infra {

intermediate_station_route parse_station_route(
    std::size_t const id, pugi::xml_node const& xml_station_route,
    soro::ptr<station>, graph const& network,
    construction_materials const& mats);

}  // namespace soro::infra
