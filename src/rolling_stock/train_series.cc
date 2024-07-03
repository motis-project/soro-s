#include "soro/rolling_stock/train_series.h"

#include <iterator>

#include "utl/verify.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/any_of.h"
#include "soro/utls/std_wrapper/find_if.h"

#include "soro/si/units.h"

namespace soro::rs {

bool train_series::key::operator<(train_series::key const& o) const {
  return number_ < o.number_ || (number_ == o.number_ && company_ < o.company_);
}

bool traction_unit::is_electric() const {
  utls::expect(!equipments_.empty(), "no equipments");

  return utls::any_of(equipments_,
                      [](auto&& e) { return e.current_.has_value(); });
}

bool traction_unit::is_diesel() const {
  utls::expect(!equipments_.empty(), "no equipments");

  return equipments_.size() == 1 && !equipments_.front().current_.has_value();
}

si::weight traction_unit::translatory_mass() const {
  return weight_ * mass_factor_;
}

traction_unit::equipment::idx traction_unit::get_equipment_idx(
    electric_configuration const c) const {
  utls::expect(!equipments_.empty(), "no equipments");

  if (!c.has_value()) {
    utls::sassert(equipments_.size() == 1, "no config, but >1 equipments");
    return equipment::idx{0};
  }

  auto const it =
      utls::find_if(equipments_, [c](auto&& e) { return e.current_ == c; });

  utl::verify(it != std::end(equipments_), "no equipment for current");

  auto const dist = std::distance(std::begin(equipments_), it);

  return equipment::idx{
      utls::narrow<traction_unit::equipment::idx::value_t>(dist)};
}

}  // namespace soro::rs