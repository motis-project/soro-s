#pragma once

#include "pugixml.hpp"

#include "soro/infrastructure/layout.h"

namespace soro::infra {

coordinates parse_coordinates(pugi::xml_node const& coord_child);

}  // namespace soro::infra
