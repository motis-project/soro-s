#pragma once

#include <utility>

#include "soro/utls/container/id_iterator.h"
#include "soro/utls/coroutine/iterator.h"
#include "soro/utls/unixtime.h"

#include "soro/base/time.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/rolling_stock/ctc.h"
#include "soro/rolling_stock/freight.h"
#include "soro/rolling_stock/train_physics.h"
#include "soro/timetable/bitfield.h"
#include "soro/timetable/sequence_point.h"

namespace soro::tt {

bool is_halt(utls::unixtime arrival, utls::unixtime departure);

struct stop_time {
  bool is_halt() const noexcept;

  utls::unixtime arrival_;
  utls::unixtime departure_;
  utls::duration min_stop_time_;
};

struct train_node : infra::route_node {
  train_node(route_node const& rn, sequence_point::optional_ptr sp);

  bool omitted() const;

  sequence_point::optional_ptr sequence_point_;
};

struct train {
  using id = uint32_t;
  using ptr = soro::ptr<train>;

  static constexpr id INVALID = std::numeric_limits<id>::max();

  struct number {
    CISTA_COMPARABLE()

    using main_t = uint32_t;
    using sub_t = uint16_t;

    main_t main_{std::numeric_limits<main_t>::max()};
    sub_t sub_{std::numeric_limits<sub_t>::max()};
  };

  rs::FreightTrain freight() const;
  bool is_freight() const;

  rs::CTC ctc() const;
  bool has_ctc() const;

  si::length path_length(infra::infrastructure const& infra) const;

  relative_time first_departure() const;
  //  relative_time last_arrival() const;

  std::size_t total_halts() const;

  bool effected_by(infra::speed_limit const& spl) const;

  infra::node::ptr get_start_node(infra::infrastructure const& infra) const;

  infra::station_route::ptr first_station_route(
      infra::infrastructure const&) const;
  infra::station_route::ptr last_station_route(
      infra::infrastructure const&) const;

  infra::interlocking_route const& first_interlocking_route(
      infra::infrastructure const&) const;
  infra::interlocking_route const& last_interlocking_route(
      infra::infrastructure const&) const;

  utls::recursive_generator<train_node> iterate(
      infra::infrastructure const& infra) const;

  auto path(infra::infrastructure const& infra) const {
    return utls::make_range(
        utls::id_iterator(std::begin(path_), &infra->interlocking_.routes_),
        utls::id_iterator(std::end(path_), &infra->interlocking_.routes_));
  }

  id id_{INVALID};
  number number_{};

  bool break_in_{false};
  bool break_out_{false};
  soro::vector<infra::interlocking_route::id> path_;
  soro::vector<sequence_point> sequence_points_;

  si::length length_;

  bitfield service_days_;
  rs::train_physics physics_;

  // deprecated
  soro::vector<stop_time> stop_times_;
};

}  // namespace soro::tt
