#include "soro/rolling_stock/rolling_stock.h"

namespace soro::rs {

traction_vehicle rolling_stock::get_traction_vehicle(
    soro::string const& series, soro::string const& owner,
    variant_id const& variant_id) const {

  std::string tsk = series.data() + std::string("-") + owner.data();  // NOLINT
  soro::string const train_series_key{tsk.data()};  // NOLINT

  auto train_series_it = train_series_.find(train_series_key);
  if (train_series_it == std::cend(train_series_)) {
    throw utl::fail(
        "Could not find traction vehicle with series '{}', owner '{}' and "
        "variant '{}'",
        series, owner, variant_id);
  }

  auto const& variants = train_series_it->second.variants_;
  auto variant_it = variants.find(variant_id);
  if (variant_it == std::cend(variants)) {
    throw utl::fail(
        "Could not find traction vehicle with series '{}', owner '{}' and "
        "variant '{}'",
        series, owner, variant_id);
  }

  return variant_it->second;
}

}  // namespace soro::rs