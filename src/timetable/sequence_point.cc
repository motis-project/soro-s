#include "soro/timetable/sequence_point.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

bool sequence_point::is_halt() const noexcept { return type_ != type::TRANSIT; }

bool sequence_point::is_halt(type const t) const noexcept { return t == type_; }

relative_time sequence_point::transit_time() const noexcept {
  return departure_;
}

bool sequence_point::is_transit() const noexcept {
  return type_ == type::TRANSIT;
}

bool sequence_point::has_transit_time() const noexcept {
  return type_ == type::TRANSIT && valid(departure_);
}

absolute_time sequence_point::absolute_arrival(
    date::year_month_day const departure_day) const noexcept {
  return relative_to_absolute(departure_day, this->arrival_);
}

absolute_time sequence_point::absolute_departure(
    date::year_month_day const departure_day) const noexcept {
  return relative_to_absolute(departure_day, this->departure_);
}

utls::optional<infra::node::idx> sequence_point::get_node_idx(
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

utls::optional<node::ptr> sequence_point::get_node(
    FreightTrain const freight, infrastructure const& infra) const {
  auto const sr = infra->station_routes_[station_route_];
  return this->get_node_idx(freight, infra).transform([&](auto&& idx) {
    return utls::optional<node::ptr>{sr->nodes(idx)};
  });
}

}  // namespace soro::tt