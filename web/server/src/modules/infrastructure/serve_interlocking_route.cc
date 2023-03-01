#include "soro/server/modules/infrastructure/infrastructure.h"

#include "range/v3/view/transform.hpp"

#include "net/web_server/responses.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"
#include "soro/utls/parse_int.h"

namespace soro::server {

net::web_server::string_res_t infra_state::serve_interlocking_route(
    net::query_router::route_request const& req) const {
  using namespace soro::infra;

  auto const infra = get_infra(req.path_params_.front());
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  auto const ir_id = utls::parse_int<station_route::id>(req.path_params_[1]);
  if (ir_id >= (**infra)->interlocking_.routes_.size()) {
    uLOG(utl::warn) << "Requesting station route " << ir_id
                    << " but there are only "
                    << (**infra)->interlocking_.routes_.size()
                    << " interlocking routes";
    return net::not_found_response(req);
  }

  auto const& ir = (**infra)->interlocking_.routes_[ir_id];
  auto gen = ir.iterate(**infra);

  json_archive archive;
  archive.add()(
      cereal::make_nvp("id", ir.id_),
      cereal::make_nvp(
          "path",
          gen | ranges::views::transform([&](auto&& rn) {
            return (**infra)->element_positions_[rn.node_->element_->id()];
          })));
  return json_response(req, archive);
}

}  // namespace soro::server
