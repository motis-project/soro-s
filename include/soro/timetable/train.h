#pragma once

#include <utility>

#include "soro/utls/unixtime.h"

#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/rolling_stock/ctc.h"
#include "soro/rolling_stock/freight.h"
#include "soro/rolling_stock/train_physics.h"

namespace soro::tt {

bool is_halt(utls::unixtime arrival, utls::unixtime departure);

struct stop_time {
  bool is_halt() const noexcept;

  utls::unixtime arrival_;
  utls::unixtime departure_;
  utls::duration min_stop_time_;
};

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

  soro::vector<stop_time> stop_times_;
};

}  // namespace soro::tt
