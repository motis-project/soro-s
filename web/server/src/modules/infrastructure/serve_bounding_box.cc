#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "cereal/types/vector.hpp"

#include "net/web_server/responses.h"

#include "soro/utls/coordinates/gps.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_bounding_box(
    net::query_router::route_request const& req) const {

  auto const infra = get_infra(req.path_params_.front());
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  auto const bbox = utls::get_bounding_box((**infra)->element_positions_);

  json_archive archive;
  archive.add()(cereal::make_nvp("boundingBox", bbox));
  return json_response(req, archive);
}

}  // namespace soro::server
