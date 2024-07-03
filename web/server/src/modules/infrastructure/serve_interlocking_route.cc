#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include <vector>

#include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"  // NOLINT

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "utl/logging.h"

#include "soro/utls/coordinates/gps.h"
#include "soro/utls/parse_int.h"

#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_interlocking_route(
    net::query_router::route_request const& req) const {
  using namespace ranges;
  using namespace soro::infra;

  auto const ctx = get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  interlocking_route::id const ir_id{
      utls::parse_int<interlocking_route::id::value_t>(req.path_params_[1])};

  if (ir_id >= (*ctx)->infra_->interlocking_.routes_.size()) {
    uLOG(utl::warn) << "Requesting interlocking route " << ir_id
                    << " but there are only "
                    << (*ctx)->infra_->interlocking_.routes_.size()
                    << " interlocking routes";
    return net::not_found_response(req);
  }

  auto const& ir = (*ctx)->infra_->interlocking_.routes_[ir_id];

  std::vector<utls::gps> path;
  for (auto const& rn : ir.iterate((*ctx)->infra_)) {
    if (rn.omitted_) {
      continue;
    }

    path.emplace_back(
        (*ctx)->positions_.elements_[rn.node_->element_->get_id()]);
  }

  json_archive archive;
  archive.add()(cereal::make_nvp("id", ir.id_), cereal::make_nvp("path", path));
  return json_response(req, archive);
}

}  // namespace soro::server
