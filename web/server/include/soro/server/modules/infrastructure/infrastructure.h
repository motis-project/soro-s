#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "range/v3/view/map.hpp"
#include "range/v3/view/transform.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/web_server.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/server/server_settings.h"

namespace soro::server {

struct infra_state {
  auto all() const {
    return infrastructures_ | ranges::views::values |
           ranges::views::transform([](auto&& p) -> auto const& { return *p; });
  }

  infra::infrastructure::optional_ptr get_infra(std::string_view const) const;

  net::web_server::string_res_t serve_infrastructure_names(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_station_names(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_station(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_station_route(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_interlocking_route(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_exclusion_set(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_exclusion_sets(
      net::query_router::route_request const& req) const;

  std::unordered_map<std::string_view,
                     std::unique_ptr<soro::infra::infrastructure>>
      infrastructures_;
};

infra_state get_infra_state(server_settings const& s);

}  // namespace soro::server
