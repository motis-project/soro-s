#pragma once

#include "soro/server/modules/infrastructure/infrastructure_module.h"
#include "soro/server/modules/timetable/timetable_module.h"

namespace soro::server {

struct ordering_module {
  net::web_server::string_res_t serve_ordering_graph(
      net::query_router::route_request const& req,
      infrastructure_module const& infra_m,
      timetable_module const& timetable_m) const;
};

ordering_module get_ordering_module();

}  // namespace soro::server
