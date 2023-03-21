#pragma once

#include "net/web_server/query_router.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"
#include "soro/server/modules/tiles/tiles_module.h"
#include "soro/server/server_settings.h"

namespace soro::server {

struct soro_server {
  explicit soro_server(server_settings const& settings);

  void run(server_settings const& settings);

private:
  void set_up_routes();

  net::query_router router_;

  infrastructure_module infrastructure_module_;
  tiles_module tiles_module_;
};

}  // namespace soro::server
