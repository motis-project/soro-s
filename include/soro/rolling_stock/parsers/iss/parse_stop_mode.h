#pragma once

#include "pugixml.hpp"

#include "soro/rolling_stock/stop_mode.h"

namespace soro::rs {

stop_mode parse_stop_mode(pugi::xml_node const& stop_mode_xml);

}  // namespace soro::rs
