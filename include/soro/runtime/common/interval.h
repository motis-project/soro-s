#pragma once

#include "soro/timetable/sequence_point.h"

namespace soro::runtime {

struct record {
  record() = delete;
  record(infra::node::ptr const node, si::length const dist)
      : node_(node), dist_(dist) {}

  auto type() const { return node_->element_->type(); }

  infra::node::ptr node_{nullptr};
  si::length dist_{si::length::zero()};
};

constexpr si::length sight_distance = si::from_m(250);

struct signal {
  CISTA_COMPARABLE()

  bool in_sight(si::length const dist) const {
    utls::expect(dist <= dist_, "distance is greater than signal distance");
    return dist_ - dist <= sight_distance;
  }

  infra::element::id id_{infra::element::id::invalid()};
  // TODO(julian) inferable from the id_ in combination with infrastructure
  infra::type type_{infra::type::INVALID};
  si::length dist_{si::length::invalid()};
};

struct interval_point {
  interval_point(si::length const dist, si::speed const limit,
                 si::slope const slope, infra::brake_path const brake_path,
                 si::length const brake_path_length,
                 tt::sequence_point::optional_ptr const sp,
                 soro::vector<record> records)
      : distance_(dist),
        limit_{limit},
        bwp_limit_{limit},
        slope_{slope},
        brake_path_{brake_path},
        brake_path_length_{brake_path_length},
        sequence_point_{sp},
        records_{std::move(records)} {}

  bool has_signal() const {
    return last_signal_.has_value() && last_signal_->dist_ == distance_;
  }

  bool has_signal(infra::type const t) const {
    return has_signal() && last_signal_->type_ == t;
  }

  // everything here is right oriented w.r.t to the point given by dist_
  si::length distance_{si::length::invalid()};

  // given by the speed limits of the infrastructure
  si::speed limit_{si::speed::invalid()};
  // given by the combination of brake paths and train brake weight percentages
  si::speed bwp_limit_{si::speed::invalid()};
  // given by the trains deacceleration and distance to the next main signal
  // determined under the assumption that the next main signal shows stop
  si::speed signal_limit_{si::speed::invalid()};

  si::slope slope_{si::slope::invalid()};
  // average slope determined by the brake path and previous slopes
  si::slope avg_slope_{si::slope::invalid()};

  infra::brake_path brake_path_{infra::brake_path::invalid()};
  si::length brake_path_length_{si::length::invalid()};

  tt::sequence_point::optional_ptr sequence_point_;

  // gives information about the last signal passed,
  // especially if it is located at this interval point
  std::optional<signal> last_signal_;
  // gives information about the next signal that's going to be passed
  std::optional<signal> next_signal_;

  soro::vector<record> records_;
};

struct interval {
  using iterator_category = typename std::contiguous_iterator_tag;
  using value_type = absolute_time;
  using difference_type = value_type;
  using pointer = value_type*;
  using reference = value_type;

  bool starts_on_stop() const;
  bool ends_on_stop() const;

  bool starts_on_signal() const;
  bool starts_on_signal(infra::type const t) const;

  bool ends_on_signal() const;
  bool ends_on_signal(infra::type const t) const;

  signal const& start_signal() const;
  signal const& end_signal() const;

  signal const& next_signal() const;

  duration min_stop_time() const;
  soro::optional<relative_time> end_departure() const;

  si::slope slope() const;
  si::slope avg_slope() const;
  si::length length() const;
  si::length start_distance() const;
  si::length end_distance() const;

  infra::brake_path brake_path() const;
  si::length brake_path_length() const;

  si::speed infra_limit() const;
  si::speed bwp_limit() const;
  si::speed speed_limit() const;
  si::speed signal_limit() const;

  si::speed speed_limit(rs::train_physics const& tp) const;

  si::speed target_speed() const;
  si::speed target_speed(rs::train_physics const& tp) const;
  si::speed target_signal_speed() const;

  tt::sequence_point::optional_ptr sequence_point() const;

  soro::vector<record> const& records() const;

  interval& operator++();
  interval& operator--();
  interval operator-(uint32_t n) const;
  interval operator+(uint32_t n) const;
  bool operator==(interval const& o) const;
  interval const& operator*() const;

  interval_point const* p1_;
  interval_point const* p2_;
};

struct intervals {
  interval begin() const;
  interval end() const;

  interval rbegin() const;
  interval rend() const;

  soro::vector<record> const& last_records() const;

  soro::vector<interval_point> p_;
};

}  // namespace soro::runtime
