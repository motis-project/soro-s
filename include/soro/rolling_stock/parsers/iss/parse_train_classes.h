#pragma once

#include "pugixml.hpp"

#include "soro/rolling_stock/train_class.h"

#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::rs {

soro::map<train_class::key, train_class> parse_train_classes(
    infra::iss_files const& iss_files);

}  // namespace soro::rs
