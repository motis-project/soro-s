#include "soro/timetable/sequence_point.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

bool sequence_point::is_halt() const noexcept { return type_ != type::TRANSIT; }

bool sequence_point::is_halt(type const t) const noexcept { return t == type_; }

relative_time sequence_point::transit_time() const noexcept {
  utls::sassert(departure_.has_value(), "no departure time value");
  return *departure_;
}

bool sequence_point::is_transit() const noexcept {
  return type_ == type::TRANSIT;
}

bool sequence_point::has_transit_time() const noexcept {
  return type_ == type::TRANSIT && departure_.has_value();
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

node::optional_idx sequence_point::get_node_idx(
    rs::FreightTrain const freight, infra::infrastructure const& infra) const {
  auto const sr = infra->station_routes_[station_route_];

  if (has_transit_time()) {
    return sr->runtime_checkpoint_;
  }

  if (is_halt()) {
    return sr->get_halt_idx(freight);
  }

  return {};
}

node::optional_ptr sequence_point::get_node(FreightTrain const freight,
                                            infrastructure const& infra) const {
  auto const sr = infra->station_routes_[station_route_];
  auto const node_idx = get_node_idx(freight, infra);
  return node_idx.has_value() ? node::optional_ptr(sr->nodes(*node_idx))
                              : node::optional_ptr(std::nullopt);
}

}  // namespace soro::tt