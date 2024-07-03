#pragma once

#include "pugixml.hpp"

#include "soro/rolling_stock/train_series.h"

#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::rs {

soro::map<train_series::key, train_series> parse_train_series(
    infra::iss_files const& iss_files, infra::dictionaries const& dicts);

}  // namespace soro::rs
