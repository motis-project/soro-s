#pragma once

#include "soro/utls/container/optional.h"

#include "soro/base/time.h"
#include "soro/infrastructure/infrastructure.h"

namespace soro::tt {

struct additional_stop {
  constexpr additional_stop() = default;

  duration2 duration_{duration2::max()};
  bool is_before_halt_{false};
};

struct sequence_point {
  CISTA_COMPARABLE()

  using ptr = soro::ptr<sequence_point>;

  using optional_ptr = soro::optional<ptr>;

  enum struct type : uint8_t { TRANSIT, OPERATIONS, PASSENGER, REQUEST };

  bool is_halt() const noexcept;
  bool is_halt(type const t) const noexcept;

  relative_time transit_time() const noexcept;
  bool is_transit() const noexcept;
  bool has_transit_time() const noexcept;

  bool is_measurable() const noexcept {
    return has_transit_time() || is_halt();
  }

  absolute_time absolute_arrival(
      date::year_month_day departure_day) const noexcept;

  absolute_time absolute_departure(
      date::year_month_day departure_day) const noexcept;

  infra::node::optional_idx get_node_idx(
      rs::FreightTrain const freight, infra::infrastructure const& infra) const;
  infra::node::optional_ptr get_node(rs::FreightTrain freight,
                                     infra::infrastructure const& infra) const;

  type type_{type::TRANSIT};

  // relative to departure day of the train at 00:00
  relative_time arrival_{INVALID<relative_time>};
  relative_time departure_{INVALID<relative_time>};
  duration2 min_stop_time_{INVALID<duration2>};

  infra::station_route::id station_route_{infra::station_route::INVALID};
};

static const std::map<char, sequence_point::type> KEY_TO_STOP_TYPE = {
    {'2', sequence_point::type::OPERATIONS},
    {'3', sequence_point::type::REQUEST},
    {'4', sequence_point::type::PASSENGER}};

}  // namespace soro::tt