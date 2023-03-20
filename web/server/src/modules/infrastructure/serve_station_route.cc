#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "range/v3/view/transform.hpp"

#include "net/web_server/responses.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"
#include "soro/utls/parse_int.h"

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_station_route(
    net::query_router::route_request const& req) const {
  using namespace soro::infra;

  auto const infra = get_infra(req.path_params_.front());
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  auto const sr_id = utls::parse_int<station_route::id>(req.path_params_[1]);
  if (sr_id >= (**infra)->station_routes_.size()) {
    uLOG(utl::warn) << "Requesting station route " << sr_id
                    << " but there are only "
                    << (**infra)->station_routes_.size() << " station routes";
    return net::not_found_response(req);
  }

  auto const& sr = (**infra)->station_routes_[sr_id];

  json_archive archive;
  archive.add()(
      cereal::make_nvp("id", sr->id_), cereal::make_nvp("name", sr->name_),
      cereal::make_nvp("ds100", sr->station_->ds100_),
      cereal::make_nvp(
          "path", sr->nodes() | ranges::views::transform([&](auto&& n) {
                    return (**infra)->element_positions_[n->element_->id()];
                  })));
  return json_response(req, archive);
}

}  // namespace soro::server
