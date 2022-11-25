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

struct stop {
  enum class type : uint8_t { NO, OPERATIONS, PASSENGER, REQUEST };

  bool is_halt() const noexcept { return type_ != type::NO; }
  bool is_halt(type const t) const noexcept { return t == type_; }

  absolute_time absolute_arrival(
      date::year_month_day const departure_day) const noexcept {
    return relative_to_absolute(departure_day, this->arrival_);
  }

  absolute_time absolute_departure(
      date::year_month_day const departure_day) const noexcept {
    return relative_to_absolute(departure_day, this->departure_);
  }

  type type_{type::NO};

  // relative to departure day of the train at 00:00
  relative_time arrival_{};
  relative_time departure_{};
  duration2 min_stop_time_{};
};

static const std::map<char, stop::type> KEY_TO_STOP_TYPE = {
    {'2', stop::type::OPERATIONS},
    {'3', stop::type::REQUEST},
    {'4', stop::type::PASSENGER}};

struct train {
  using id = soro::size_type;
  using ptr = soro::ptr<train>;

  static constexpr id INVALID = std::numeric_limits<id>::max();

  bool freight() const;
  bool ctc() const;

  si::length path_length(infra::infrastructure const& infra) const;

  utls::unixtime first_departure() const;
  utls::unixtime last_arrival() const;

  std::size_t total_halts() const;

  infra::node::ptr get_start_node() const;
  infra::node::ptr get_end_node() const;

  infra::station_route::ptr first_station_route(
      infra::infrastructure const&) const;

  utls::recursive_generator<infra::route_node> iterate(
      infra::skip_omitted skip, infra::infrastructure const& infra) const;

  bool has_event_in_interval(utls::unixtime start, utls::unixtime end) const;

  id id_{INVALID};
  soro::string name_;

  rs::train_physics physics_;
  soro::vector<infra::interlocking_route::id> path_;

  rs::FreightTrain freight_{rs::FreightTrain::NO};
  rs::CTC ctc_{rs::CTC::NO};

  bitfield bitfield_;
  soro::vector<stop> stops;
  soro::vector<stop_time> stop_times_;

  bool break_in_;
  bool break_out_;
};

}  // namespace soro::tt
