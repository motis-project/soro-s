#pragma once

#include <map>
#include <utility>

#include "soro/utls/unixtime.h"

#include "soro/base/time.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/rolling_stock/ctc.h"
#include "soro/rolling_stock/freight.h"
#include "soro/rolling_stock/train_physics.h"
#include "soro/timetable/bitfield.h"

namespace soro::tt {

bool is_halt(utls::unixtime arrival, utls::unixtime departure);

struct stop_time {
  bool is_halt() const noexcept;

  utls::unixtime arrival_;
  utls::unixtime departure_;
  utls::duration min_stop_time_;
};

struct sequence_point {
  enum class type : uint8_t { TRANSIT, OPERATIONS, PASSENGER, REQUEST };

  bool is_halt() const noexcept;
  bool is_halt(type const t) const noexcept;

  relative_time transit_time() const noexcept;
  bool is_transit() const noexcept;
  bool has_transit_time() const noexcept;

  bool is_measurable() const noexcept { return has_transit_time() || is_halt(); }

  absolute_time absolute_arrival(
      date::year_month_day departure_day) const noexcept;

  absolute_time absolute_departure(
      date::year_month_day departure_day) const noexcept;

  utls::optional<infra::node::ptr> get_node(
      rs::FreightTrain freight, infra::infrastructure const& infra) const;

  type type_{type::TRANSIT};

  // relative to departure day of the train at 00:00
  relative_time arrival_{INVALID_RELATIVE_TIME};
  relative_time departure_{INVALID_RELATIVE_TIME};
  duration2 min_stop_time_{INVALID_DURATION};

  infra::station_route::id station_route_{infra::station_route::INVALID};
};

struct stop_sequence {
  std::vector<sequence_point> points_;
  bool break_in_;
  bool break_out_;
};

static const std::map<char, sequence_point::type> KEY_TO_STOP_TYPE = {
    {'2', sequence_point::type::OPERATIONS},
    {'3', sequence_point::type::REQUEST},
    {'4', sequence_point::type::PASSENGER}};

struct train {
  using id = soro::size_type;
  using ptr = soro::ptr<train>;

  static constexpr id INVALID = std::numeric_limits<id>::max();

  struct number {
    CISTA_COMPARABLE()

    using main_t = uint32_t;
    using sub_t = uint16_t;

    main_t main_{std::numeric_limits<main_t>::max()};
    sub_t sub_{std::numeric_limits<sub_t>::max()};
  };

  struct path {
    struct entry {
      soro::vector<sequence_point> sequence_points_;
      infra::interlocking_route::id interlocking_id_;
    };

    bool break_in_{false};
    bool break_out_{false};
    soro::vector<entry> entries_;

    si::length length_{si::INVALID<si::length>};
  };

  rs::FreightTrain freight() const;
  bool is_freight() const;

  rs::CTC ctc() const;
  bool has_ctc() const;

  si::length path_length(infra::infrastructure const& infra) const;

  relative_time first_departure() const;
  //  relative_time last_arrival() const;

  std::size_t total_halts() const;

  infra::node::ptr get_start_node(infra::infrastructure const& infra) const;

  infra::station_route::ptr first_station_route(
      infra::infrastructure const&) const;
  infra::interlocking_route const& first_interlocking_route(
      infra::infrastructure const&) const;

  utls::recursive_generator<infra::route_node> iterate(
      infra::infrastructure const& infra) const;

  //  bool has_event_in_interval(utls::unixtime start, utls::unixtime end)
  //  const;

  id id_{INVALID};
  number number_{};

  path path_;
  bitfield service_days_;
  rs::train_physics physics_;

  // deprecated
  soro::vector<stop_time> stop_times_;
};

}  // namespace soro::tt
