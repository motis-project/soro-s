#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "cereal/cereal.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/infrastructure/station/station.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

namespace soro::infra {

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive, station::ptr const s) {
  archive(cereal::make_nvp("ds100", s->ds100_), cereal::make_nvp("id", s->id_));
}

}  // namespace soro::infra

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_station_names(
    net::query_router::route_request const& req) const {

  auto const ctx = get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  json_archive archive;
  archive.add()(cereal::make_nvp("stations", (*ctx)->infra_->stations_));
  return json_response(req, archive);
}

}  // namespace soro::server
