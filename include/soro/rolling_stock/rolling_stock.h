#pragma once

#include <map>
#include <vector>

#include "soro/base/soro_types.h"
#include "soro/rolling_stock/train_series.h"
#include "soro/utls/file/loaded_file.h"

namespace soro::rs {

struct rolling_stock {
  traction_vehicle get_traction_vehicle(std::string_view const series,
                                        std::string_view const owner,
                                        variant_id const variant_id) const;

  soro::map<soro::string, train_series> train_series_;
};

rolling_stock parse_rolling_stock(
    std::vector<utls::loaded_file> const& core_files);

}  // namespace soro::rs
