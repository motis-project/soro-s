#pragma once

#include "soro/rolling_stock/rolling_stock.h"

#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::rs {

rolling_stock parse_rolling_stock(infra::iss_files const& iss_files,
                                  infra::dictionaries const& dicts);

}  // namespace soro::rs
