#pragma once

#include "soro/base/soro_types.h"

#include "soro/utls/result.h"

#include "soro/rolling_stock/train_category.h"
#include "soro/rolling_stock/train_class.h"
#include "soro/rolling_stock/train_series.h"

namespace soro::rs {

struct rolling_stock {
  soro::map<train_series::key, train_series> train_series_;
  soro::map<train_class::key, train_class> train_classes_;
  soro::map<train_category::key, train_category> train_categories_;
};

utls::result<traction_unit> get_traction_vehicle(rolling_stock const& rs,
                                                 traction_unit::key const& tuk);

}  // namespace soro::rs
