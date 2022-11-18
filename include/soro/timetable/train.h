#pragma once

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

struct id2 {
  using base_train_id = uint32_t;
  using bitfield_idx = uint16_t;

  base_train_id base_id_{std::numeric_limits<base_train_id>::max()};
  bitfield_idx bit_offset_{std::numeric_limits<bitfield_idx>::max()};
};

struct train {
  using id = soro::size_type;
  using ptr = soro::ptr<train>;

  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr id2 INVALID2 = id2();

  bool freight() const;
  bool ctc() const;
  si::length path_length() const;

  utls::unixtime first_departure() const;
  utls::unixtime last_arrival() const;

  std::size_t total_halts() const;

  infra::node_ptr get_start_node() const;
  infra::node_ptr get_end_node() const;

  utls::recursive_generator<infra::route_node> iterate(
      infra::skip_omitted skip) const;

  bool has_event_in_interval(utls::unixtime start, utls::unixtime end) const;

  id id_{INVALID};
  soro::string name_;

  rs::train_physics physics_;
  soro::vector<infra::interlocking_route::ptr> path_;

  rs::FreightTrain freight_{rs::FreightTrain::NO};
  rs::CTC ctc_{rs::CTC::NO};

  bitfield bitfield_;
  soro::vector<stop> stops;
  soro::vector<stop_time> stop_times_;
};

}  // namespace soro::tt
