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

#include "soro/server/modules/infrastructure/positions.h"

namespace soro::server {

struct infrastructure_module {
  struct context {
    using ptr = soro::ptr<context>;

    infra::infrastructure infra_;
    positions positions_;
  };

  auto all() const { return contexts_ | ranges::views::values; }

  soro::optional<infrastructure_module::context::ptr> get_context(
      std::string const& context_name) const;

  net::web_server::string_res_t serve_infrastructure_names(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_bounding_box(
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

  net::web_server::string_res_t serve_element(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_node(
      net::query_router::route_request const& req) const;

  std::unordered_map<std::string, context> contexts_;

  // shared among all contexts
  soro::map<infra::station::ds100, utls::gps> station_positions_;
};

infrastructure_module get_infrastructure_module(server_settings const& s);

}  // namespace soro::server
