#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "cereal/cereal.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/utls/coordinates/gps.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_bounding_box(
    net::query_router::route_request const& req) const {

  auto const context = get_context(req.path_params_.front());
  if (!context.has_value()) {
    return net::not_found_response(req);
  }

  auto const bbox = utls::get_bounding_box((*context)->positions_.elements_);

  json_archive archive;
  archive.add()(cereal::make_nvp("boundingBox", bbox));
  return json_response(req, archive);
}

}  // namespace soro::server
