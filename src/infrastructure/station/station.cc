#include "soro/infrastructure/station/station.h"

#include "range/v3/action/sort.hpp"
#include "range/v3/action/unique.hpp"
#include "range/v3/range/conversion.hpp"
#include "range/v3/view/transform.hpp"

namespace soro::infra {

soro::vector<station::ptr> station::neighbours() const {
  return borders_ |
         ranges::views::transform([](auto&& b) { return b.neighbour_; }) |
         ranges::to<soro::vector<station::ptr>>() | ranges::actions::sort |
         ranges::actions::unique;
}

}  // namespace soro::infra