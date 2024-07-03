#pragma once

#include "soro/utls/container/optional.h"

#include "soro/base/time.h"

#include "soro/infrastructure/infrastructure.h"

namespace soro::tt {

struct sequence_point {
  CISTA_COMPARABLE()

  using ptr = soro::ptr<sequence_point>;
  using optional_ptr = soro::optional<ptr>;

  // TRANSIT is when the train is not required to stop moving
  // the rest of them are stops (i.e. they require the train to stop moving)
  // OPERATIONS, PASSENGER, REQUEST are halts, they require the train to stop
  // and are executed at halt nodes
  // ADDITIONAL requires the train to stop moving, but can be executed
  // at arbitrary nodes, there are no times for these in the timetable
  enum struct type : uint8_t {
    TRANSIT,
    OPERATIONS,
    PASSENGER,
    REQUEST,
    ADDITIONAL,
    INVALID
  };

  bool is_halt() const noexcept;
  bool is_stop() const noexcept;
  bool is_transit() const noexcept;
  bool is_halt_type(type const t) const noexcept;

  bool is_measurable() const noexcept;

  duration min_stop_time() const noexcept;
  duration stop_time() const noexcept;

  soro::optional<absolute_time> absolute_arrival(
      date::year_month_day departure_day) const noexcept;
  soro::optional<absolute_time> absolute_departure(
      date::year_month_day departure_day) const noexcept;

  soro::optional<absolute_time> absolute_arrival(
      absolute_time const midnight) const noexcept;
  soro::optional<absolute_time> absolute_departure(
      absolute_time const midnight) const noexcept;

  infra::node::ptr get_node(infra::infrastructure const& infra) const;

  type type_{type::INVALID};

  // relative to departure day of the train at 00:00
  // do not necessarily exist, e.g. transits without runtime checkpoint
  // and additional stops
  soro::optional<relative_time> arrival_;
  soro::optional<relative_time> departure_;

  // zero for transits
  duration min_stop_time_{duration::max()};

  infra::station_route::idx idx_{infra::station_route::invalid_idx()};
  infra::station_route::id station_route_{infra::station_route::invalid()};
};

static const std::map<char, sequence_point::type> KEY_TO_STOP_TYPE = {
    {'2', sequence_point::type::OPERATIONS},
    {'3', sequence_point::type::REQUEST},
    {'4', sequence_point::type::PASSENGER}};

}  // namespace soro::tt