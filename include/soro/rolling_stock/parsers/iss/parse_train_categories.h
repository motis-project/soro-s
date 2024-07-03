#pragma once

#include "pugixml.hpp"

#include "soro/rolling_stock/train_category.h"

#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::rs {

soro::map<train_category::key, train_category> parse_train_categories(
    infra::iss_files const& iss_files);

}  // namespace soro::rs
