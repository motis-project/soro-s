#pragma once

#include "pugixml.hpp"

#include "soro/rolling_stock/train_type.h"

namespace soro::rs {

train_type parse_train_type(pugi::xml_node const& train_type_xml);

}  // namespace soro::rs
