#pragma once

#include "soro/utls/coordinates/gps.h"

#include "soro/infrastructure/brake_tables.h"
#include "soro/infrastructure/critical_section.h"
#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/exclusion/exclusion.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/infrastructure_options.h"
#include "soro/infrastructure/interlocking/interlocking.h"
#include "soro/infrastructure/layout.h"
#include "soro/infrastructure/line.h"
#include "soro/infrastructure/station/station.h"
#include "soro/infrastructure/station/station_route_graph.h"
#include "soro/rolling_stock/rolling_stock.h"

namespace soro::infra {

struct version {
  CISTA_COMPARABLE()

  using number = uint32_t;

  soro::string name_;
  number number_{0};
};

struct default_values {
  soro::string line_class_{"invalid"};
  si::time route_form_time_{si::time::invalid()};
  si::length brake_path_length_{si::length::invalid()};
  brake_path brake_path_{brake_path::invalid()};
  si::speed stationary_speed_limit_{si::speed::invalid()};
};

struct infrastructure_t {
  graph graph_;

  default_values defaults_;
  rs::rolling_stock rolling_stock_;

  soro::vector_map<station::id, station::ptr> stations_;
  soro::vector_map<station_route::id, station_route::ptr> station_routes_;
  soro::vector<station_route::path::ptr> station_route_paths_;

  soro::map<soro::string, station::ptr> ds100_to_station_;
  soro::map<element::id, station::ptr> element_to_station_;

  station_route_graph station_route_graph_;

  interlocking interlocking_;
  exclusion exclusion_;

  critical_sections critical_sections_;

  brake_tables brake_tables_;

  soro::vector_map<station::id, soro::string> full_station_names_;

  layout layout_;

  lines lines_;

  soro::vector<soro::unique_ptr<station>> station_store_;
  soro::vector<soro::unique_ptr<station_route>> station_route_store_;
  soro::vector<soro::unique_ptr<station_route::path>> station_route_path_store_;

  soro::string source_;
  version version_;

  dictionaries dictionaries_;
};

}  // namespace soro::infra
