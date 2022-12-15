#pragma once

#include "soro/utls/coordinates/gps.h"

#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/infrastructure_options.h"
#include "soro/infrastructure/interlocking/interlocking_subsystem.h"
#include "soro/infrastructure/station/station.h"
#include "soro/infrastructure/station/station_route_graph.h"
#include "soro/rolling_stock/rolling_stock.h"

namespace soro::infra {

struct default_values {
  soro::string line_class_;
  si::time route_form_time_{si::INVALID<si::time>};
  si::length brake_path_length_{si::INVALID<si::length>};
  si::speed stationary_speed_limit_{si::INVALID<si::speed>};
};

struct infrastructure_t {
  using ptr = soro::ptr<infrastructure_t>;

  graph graph_;

  default_values defaults_;
  rs::rolling_stock rolling_stock_;

  soro::vector<station::ptr> stations_{};
  soro::vector<station_route::ptr> station_routes_{};
  soro::vector<station_route::path::ptr> station_route_paths_{};

  soro::map<soro::string, station::ptr> ds100_to_station_{};
  soro::map<element_id, station::ptr> element_to_station_{};

  station_route_graph station_route_graph_{};

  interlocking_subsystem interlocking_{};

  soro::vector<soro::string> full_station_names_{};

  soro::vector<utls::gps> station_positions_{};
  soro::vector<utls::gps> element_positions_{};

  soro::vector<soro::unique_ptr<station>> station_store_{};
  soro::vector<soro::unique_ptr<station_route>> station_route_store_{};
  soro::vector<soro::unique_ptr<station_route::path>>
      station_route_path_store_{};

  soro::string source_{};
};

}  // namespace soro::infra
