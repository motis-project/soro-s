#pragma once

#include "range/v3/range/concepts.hpp"
#include "range/v3/view/empty.hpp"

#include "soro/timetable/timetable.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"
#include "soro/server/server_settings.h"

namespace soro::server {

struct timetable_module {
  struct infra_context {
    std::unordered_map<std::string, std::unique_ptr<tt::timetable>> timetables_;
  };

  // in header file, because auto is really convenient here ...
  auto all(std::string const& infrastructure_source) const {
    // helper to return an empty range when the infrastructure is not found
    static auto empty_context = infra_context{};

    auto context_it = contexts_.find(infrastructure_source);

    auto const& context =
        context_it == std::end(contexts_) ? empty_context : context_it->second;

    return context.timetables_ | ranges::views::values |
           ranges::views::transform([](auto&& p) -> auto const& { return *p; });
  }

  tt::timetable::optional_ptr get_timetable(
      std::string const& infrastructure_name,
      std::string const& timetable_name) const;

  net::web_server::string_res_t serve_timetable_names(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_timetable(
      net::query_router::route_request const& req) const;

  net::web_server::string_res_t serve_intervals(
      net::query_router::route_request const& req,
      infrastructure_module const& infra_m) const;

  std::unordered_map<std::string, infra_context> contexts_;
};

timetable_module get_timetable_module(server_settings const& s,
                                      infrastructure_module const& infra_m);

}  // namespace soro::server
