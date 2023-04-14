#include "soro/rolling_stock/rolling_stock.h"

#include "soro/rolling_stock/rolling_stock_error.h"

namespace soro::rs {

utls::result<traction_vehicle> rolling_stock::get_traction_vehicle(
    std::string_view const series, std::string_view const owner,
    variant_id variant_id) const {

  std::string tsk = series.data() + std::string("-") + owner.data();  // NOLINT
  soro::string const train_series_key{tsk.data()};  // NOLINT

  auto train_series_it = train_series_.find(train_series_key);
  if (train_series_it == std::cend(train_series_)) {
    return utls::unexpected(error::rolling_stock::TRACTION_VEHICLE_NOT_FOUND,
                            "could not find traction vehicle with series '{}', "
                            "owner '{}' and variant '{}', due to train series",
                            series, owner, variant_id);
  }

  auto const& variants = train_series_it->second.variants_;
  auto variant_it = variants.find(variant_id);
  if (variant_it == std::cend(variants)) {
    return utls::unexpected(error::rolling_stock::TRACTION_VEHICLE_NOT_FOUND,
                            "could not find traction vehicle with series '{}', "
                            "owner '{}' and variant '{}', due to variant",
                            series, owner, variant_id);
  }

  return variant_it->second;
}

}  // namespace soro::rs