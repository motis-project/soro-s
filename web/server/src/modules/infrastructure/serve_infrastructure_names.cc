#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "range/v3/view/map.hpp"

#include "net/web_server/responses.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_infrastructure_names(
    net::query_router::route_request const& req) const {
  json_archive archive;
  archive.add()(cereal::make_nvp("infrastructures",
                                 infrastructures_ | ranges::views::keys));
  return json_response(req, archive);
}

}  // namespace soro::server
