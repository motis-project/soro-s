#pragma once

#include "net/web_server/query_router.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"
#include "soro/server/modules/tiles/tiles_module.h"
#include "soro/server/modules/search/search_module.h"
#include "soro/server/server_settings.h"

namespace soro::server {

struct soro_server {
  explicit soro_server(server_settings const& s);

  void run(server_settings const& s);

private:
  void set_up_routes(server_settings const& s);

  net::query_router router_;

  infrastructure_module infrastructure_module_;
  tiles_module tiles_module_;
  search_module search_module_;
};

}  // namespace soro::server
