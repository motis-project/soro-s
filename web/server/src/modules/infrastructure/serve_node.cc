#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "cereal/cereal.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/base/soro_types.h"

#include "soro/utls/parse_int.h"

#include "soro/infrastructure/graph/node.h"

#include "soro/server/cereal/json_archive.h"

#include "soro/server/modules/infrastructure/serialize_element.h"

namespace soro::server {

using namespace soro::infra;

net::web_server::string_res_t infrastructure_module::serve_node(
    net::query_router::route_request const& req) const {
  auto const ctx = get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  node::id const n_id{utls::parse_int<node::id::value_t>(req.path_params_[1])};
  auto const& node = (*ctx)->infra_->graph_.nodes_[n_id];

  json_archive archive;

  archive.add()(cereal::make_nvp("nodeId", as_val(n_id)));

  serialize_element(archive, node->element_, (*ctx)->infra_);

  return json_response(req, archive);
}

}  // namespace soro::server
