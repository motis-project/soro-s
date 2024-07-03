#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"  // NOLINT

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/utls/parse_int.h"

#include "soro/infrastructure/exclusion/exclusion_set.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

using namespace soro::infra;

net::web_server::string_res_t infrastructure_module::serve_exclusion_set(
    net::query_router::route_request const& req) const {
  auto const ctx = get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  auto const es_id = utls::parse_int<exclusion_set::id>(req.path_params_[1]);
  auto const& es = (*ctx)->infra_->exclusion_.exclusion_sets_[es_id];

  json_archive archive;
  archive.add()(cereal::make_nvp("id", es_id),
                cereal::make_nvp("size", es.size()),
                cereal::make_nvp("interlocking_routes", es));
  return json_response(req, archive);
}

}  // namespace soro::server
