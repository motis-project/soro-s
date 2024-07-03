#pragma once

#include "soro/infrastructure/graph/element_data.h"
#include "soro/timetable/train.h"

namespace soro::runtime {

struct active_speed_limit {
  active_speed_limit(si::length const dist, si::length const original_dist,
                     infra::speed_limit::ptr const spl);

  active_speed_limit(infra::speed_limit::ptr const spl, si::length const dist);

  bool operator==(active_speed_limit const&) const = default;
  bool operator<(auto const& o) const;

  si::speed speed() const;

  si::length dist_{si::length::invalid()};
  si::length original_dist_{si::length::invalid()};
  infra::speed_limit::ptr limit_{nullptr};
};

struct speed_limit_manager {
  speed_limit_manager(tt::train const& train,
                      infra::infrastructure const& infra);

  bool add(active_speed_limit const& new_spl);

  // determine the current speed limit
  si::speed get_current_limit() const;
  void add_initial_speed_limit(tt::train const& train,
                               infra::speed_limit::ptr const spl,
                               si::length const dist);

  void initialize_default(infra::default_values const& defaults);

  void initialize_from_infra(tt::train const& train,
                             infra::infrastructure const& infra,
                             si::length current_dist);

  si::length initialize_from_sr(tt::train const& train,
                                infra::infrastructure const& infra);

  void initialize(tt::train const& train, infra::infrastructure const& infra);

private:
  // the next functions are for adding the new speed limit to the active speed
  // limits
  bool add_here_general(active_speed_limit const& new_spl);

  bool add_here_special(active_speed_limit const& new_spl);

  bool add_station_route_infinite(active_speed_limit const& new_spl);

  bool add_station_route_length(active_speed_limit const& new_spl);

  std::optional<active_speed_limit> get_wirkende_fwg1() const;

  bool add_station_route_length_end(active_speed_limit const& new_spl);

  bool add_station_route_length_based(active_speed_limit const& new_spl);

  bool add_last_signal_general(active_speed_limit const& new_spl);

  bool add_last_signal_special(active_speed_limit const& new_spl);

  bool add_ends_special(active_speed_limit const& new_spl);

  // the next functions are for dispatching the new speed limit to the correct
  // handler
  bool add_infrastructure(active_speed_limit const& new_spl);

  bool add_station_route(active_speed_limit const& new_spl);

  std::optional<active_speed_limit> wirkende_g1_;

  // alle wirkenden s1
  soro::vector<active_speed_limit> s1s_;

  // alle wirkenden fws1
  soro::vector<active_speed_limit> fws1s_;

  // alle wirkende fwg1s mit länge
  soro::vector<active_speed_limit> active_fwg1_length_;

  // wirkende fwg1 unendliche laenge
  std::optional<active_speed_limit> fwg1_infinite_;

  // alle wirkenden s2
  soro::vector<active_speed_limit> s2s_;

  // alle wirkenden g2
  soro::vector<active_speed_limit> g2s_;

  // we need these for the process of finding the initial speed limits
  infra::speed_limit default_;
  soro::vector<active_speed_limit> initial_special_ends_;
};

}  // namespace soro::runtime
