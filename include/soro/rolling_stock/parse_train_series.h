#pragma once

#include "pugixml.hpp"

#include "soro/rolling_stock/train_series.h"

namespace soro::rs {

soro::vector<train_series> parse_train_series(
    pugi::xml_node const& xml_train_model_ranges);

}  // namespace soro::rs
