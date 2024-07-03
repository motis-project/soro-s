#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/utls/parse_int.h"

#include "soro/infrastructure/graph/element.h"

#include "soro/server/cereal/json_archive.h"

#include "soro/server/modules/infrastructure/serialize_element.h"

namespace soro::server {

using namespace soro::infra;

net::web_server::string_res_t infrastructure_module::serve_element(
    net::query_router::route_request const& req) const {
  auto const ctx = get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  element::id const e_id{
      utls::parse_int<element::id::value_t>(req.path_params_[1])};
  auto const& element = (*ctx)->infra_->graph_.elements_[e_id];

  json_archive archive;

  serialize_element(archive, element, (*ctx)->infra_);

  return json_response(req, archive);
}

}  // namespace soro::server
