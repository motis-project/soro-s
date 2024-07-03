#include "soro/rolling_stock/rolling_stock.h"

#include <iterator>

#include "soro/base/soro_types.h"

#include "soro/utls/result.h"

#include "soro/rolling_stock/rolling_stock_error.h"
#include "soro/rolling_stock/train_series.h"

namespace soro::rs {

utls::result<traction_unit> get_traction_vehicle(
    rolling_stock const& rs, traction_unit::key const& tuk) {
  auto const train_series_it = rs.train_series_.find(tuk.train_series_key_);
  if (train_series_it == std::cend(rs.train_series_)) {
    return utls::unexpected(error::rolling_stock::TRACTION_VEHICLE_NOT_FOUND,
                            "could not find traction vehicle with series '{}', "
                            "owner '{}' and variant '{}', due to train series",
                            tuk.train_series_key_.number_,
                            tuk.train_series_key_.company_,
                            as_val(tuk.variant_));
  }

  auto const& variants = train_series_it->second.variants_;
  auto const variant_it = variants.find(tuk.variant_);
  if (variant_it == std::cend(variants)) {
    return utls::unexpected(error::rolling_stock::TRACTION_VEHICLE_NOT_FOUND,
                            "could not find traction vehicle with series '{}', "
                            "owner '{}' and variant '{}', due to variant",
                            tuk.train_series_key_.number_,
                            tuk.train_series_key_.company_,
                            as_val(tuk.variant_));
  }

  return variant_it->second;
}

}  // namespace soro::rs