#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "cereal/types/vector.hpp"

#include "net/web_server/responses.h"

#include "soro/server/cereal/json_archive.h"
#include "soro/utls/parse_int.h"

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_exclusion_set(
    net::query_router::route_request const& req) const {
  using namespace soro::infra;

  auto const infra = get_infra(req.path_params_.front());
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  auto const es_id = utls::parse_int<exclusion_set::id>(req.path_params_[1]);
  auto const& es = (**infra)->exclusion_.exclusion_sets_[es_id];

  json_archive archive;
  archive.add()(cereal::make_nvp("id", es_id),
                cereal::make_nvp("size", es.size()),
                cereal::make_nvp("interlocking_routes", es));
  return json_response(req, archive);
}

}  // namespace soro::server
