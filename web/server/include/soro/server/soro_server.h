#pragma once

#include "net/web_server/query_router.h"

#include "soro/server/modules/infrastructure/infrastructure.h"
#include "soro/server/modules/tiles/tiles.h"
#include "soro/server/server_settings.h"

namespace soro::server {

struct soro_server {
  explicit soro_server(server_settings const& settings);

private:
  void set_up_routes();

  net::query_router router_;

  infra_state infra_state_;
  tiles tiles_;
};

}  // namespace soro::server
