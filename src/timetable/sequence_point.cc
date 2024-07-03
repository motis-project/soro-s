#include "soro/timetable/sequence_point.h"

#include "date/date.h"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/any.h"
#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/station/station_route.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

bool sequence_point::is_halt() const noexcept {
  return type_ == utls::any{type::PASSENGER, type::REQUEST, type::OPERATIONS};
}

bool sequence_point::is_stop() const noexcept {
  return type_ == type::ADDITIONAL || is_halt();
}

bool sequence_point::is_halt_type(type const t) const noexcept {
  return t == type_;
}

bool sequence_point::is_transit() const noexcept {
  return type_ == type::TRANSIT;
}

bool sequence_point::is_measurable() const noexcept {
  return arrival_.has_value() && departure_.has_value();
}

duration sequence_point::min_stop_time() const noexcept {
  return min_stop_time_;
}

duration sequence_point::stop_time() const noexcept {
  return is_measurable() ? *departure_ - *arrival_ : duration::zero();
}

soro::optional<absolute_time> sequence_point::absolute_arrival(
    date::year_month_day const departure_day) const noexcept {
  absolute_time const midnight_departure = date::sys_days{departure_day};
  utls::sassert(midnight_departure == midnight(midnight_departure));
  return this->absolute_arrival(midnight_departure);
}

soro::optional<absolute_time> sequence_point::absolute_departure(
    date::year_month_day const departure_day) const noexcept {
  absolute_time const midnight_departure = date::sys_days{departure_day};
  utls::sassert(midnight_departure == midnight(midnight_departure));
  return this->absolute_departure(midnight_departure);
}

soro::optional<absolute_time> sequence_point::absolute_arrival(
    absolute_time const midnight) const noexcept {
  utls::expect(midnight == soro::midnight(midnight),
               "anchor is not midnight, but {}.", midnight);
  return arrival_.transform(
      [&midnight](auto&& arr) { return relative_to_absolute(midnight, arr); });
}

soro::optional<absolute_time> sequence_point::absolute_departure(
    absolute_time const midnight) const noexcept {
  utls::expect(midnight == soro::midnight(midnight),
               "anchor is not midnight, but {}.", midnight);
  return departure_.transform(
      [&midnight](auto&& dep) { return relative_to_absolute(midnight, dep); });
}

node::ptr sequence_point::get_node(infrastructure const& infra) const {
  utls::expect(station_route_ != station_route::invalid(),
               "no station route value");
  utls::expect(idx_ != station_route::invalid_idx(), "no node index value");

  return infra->station_routes_[station_route_]->nodes(idx_);
}

}  // namespace soro::tt