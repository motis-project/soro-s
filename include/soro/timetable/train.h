#pragma once

#include <limits>

#include "soro/base/time.h"

#include "soro/utls/container/it_range.h"

#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "soro/rolling_stock/safety_systems.h"
#include "soro/rolling_stock/train_physics.h"

#include "soro/timetable/bitfield.h"
#include "soro/timetable/interval.h"
#include "soro/timetable/sequence_point.h"

namespace soro::tt {

// yielded when iterating a train
struct train_node : infra::route_node {
  train_node(route_node const& rn, sequence_point::optional_ptr const sp);

  bool omitted() const;

  sequence_point::optional_ptr sequence_point_;
};

struct train {
  using id = uint32_t;
  using ptr = soro::ptr<train>;

  static id invalid() { return std::numeric_limits<id>::max(); }

  struct number {
    CISTA_COMPARABLE()

    using main_t = uint32_t;
    using sub_t = uint16_t;

    main_t main_{std::numeric_limits<main_t>::max()};
    sub_t sub_{std::numeric_limits<sub_t>::max()};
  };

  struct trip {
    using id = cista::strong<uint32_t, struct _trip_id>;
    static id invalid() { return std::numeric_limits<id>::max(); }

    trip(id const id, train::id const train_id, absolute_time const anchor)
        : id_{id}, train_id_{train_id}, anchor_{anchor} {}

    id id_{invalid()};
    train::id train_id_{tt::train::invalid()};
    absolute_time anchor_{soro::INVALID<absolute_time>};
  };

  struct iterator {
    using iterator_category = typename std::input_iterator_tag;
    using value_type = absolute_time;
    using difference_type = value_type;
    using pointer = value_type*;
    using reference = value_type;

    iterator(train const* train, interval const& interval, bool const end);

    void advance();

    iterator& operator++();

    value_type operator*() const;

    bool operator==(iterator const& other) const = default;
    bool operator!=(iterator const& other) const = default;

    train const* train_;
    bitfield::iterator bitfield_it_;
    interval interval_;
  };

  utls::it_range<iterator> departures(interval const& interval) const;
  utls::it_range<iterator> departures() const;

  rs::train_type type() const;

  rs::stop_mode stop_mode() const;
  bool uses_freight() const;

  rs::LZB lzb() const;
  bool has_lzb() const;

  si::length path_length(infra::infrastructure const& infra) const;

  absolute_time first_absolute_timestamp() const;
  absolute_time last_absolute_timestamp() const;

  interval event_interval(absolute_time const midnight) const;

  soro::size_t total_halts() const;
  soro::size_t total_stops() const;
  soro::size_t trip_count() const;

  soro::vector<trip> trips(trip::id const start_id) const;
  soro::size_t trip_count(interval const& interval) const;

  bool affected_by(infra::speed_limit const& spl) const;

  rs::surcharge_factor surcharge_factor(
      si::speed const current_max_velocity) const;

  infra::node::ptr get_start_node(infra::infrastructure const& infra) const;
  infra::node::ptr get_end_node(infra::infrastructure const& infra) const;

  bool goes_through(infra::station::id, infra::infrastructure const&) const;

  std::span<sequence_point const> get_sequence_points(
      infra::station_route::id) const;
  std::span<sequence_point const> get_sequence_points(
      infra::station::id, infra::infrastructure const&) const;

  infra::station_route::id first_station_route_id() const;
  infra::station_route::id last_station_route_id() const;

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

  id id_{invalid()};
  number number_{};

  bool break_in_{false};
  bool break_out_{false};

  // initialize to zero, so we can determine the value for braking trains
  relative_time start_time_{relative_time::zero()};
  relative_time end_time_{relative_time::zero()};

  si::speed start_speed_{si::speed::invalid()};

  soro::vector<infra::interlocking_route::id> path_;
  soro::vector<sequence_point> sequence_points_;

  bitfield service_days_;
  rs::train_physics physics_;
};

}  // namespace soro::tt

namespace fmt {

template <>
struct formatter<soro::tt::train::trip> {
  constexpr auto parse(format_parse_context& ctx)  // NOLINT
      -> decltype(ctx.begin()) {
    if (ctx.begin() != ctx.end() && *ctx.begin() != '}') {
      throw format_error("invalid format for train::trip");
    }

    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(soro::tt::train::trip const& trip,
              FormatContext& ctx) const -> decltype(ctx.out()) {
    return format_to(ctx.out(), "[train {} @ {}]", trip.train_id_,
                     date::format("%F", trip.anchor_));
  }
};

}  // namespace fmt
