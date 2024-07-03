#pragma once

#include "guess/guesser.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"

namespace soro::server {

struct search_module {
  struct result {
    enum class type : uint8_t {
      STATION,
      ELEMENT,
      STATION_ROUTE,
      INTERLOCKING_ROUTE
    };

    static std::string_view type_to_string(type const t) {
      switch (t) {
        case type::STATION: return "station";
        case type::ELEMENT: return "element";
        case type::STATION_ROUTE: return "stationRoute";
        case type::INTERLOCKING_ROUTE: return "interlockingRoute";
      }

      return "not reachable";
    }

    type type_;
    std::string name_{};
    utls::bounding_box bounding_box_;
    // TODO(julian) this could be a typesafe union / std::variant
    // now ids (station, station route, interlocking route) decay to std::size_t
    std::size_t id_{std::numeric_limits<std::size_t>::max()};
    std::string element_type_{};
  };

  struct context {
    std::vector<result> results_;
    std::unique_ptr<guess::guesser> guesser_;
  };

  net::web_server::string_res_t serve_search(
      net::query_router::route_request const& req,
      infrastructure_module const& infra_m) const;

  // infrastructure name -> search context
  std::unordered_map<std::string_view, context> contexts_;
};

search_module get_search_module(infrastructure_module const& infra_m);

}  // namespace soro::server
