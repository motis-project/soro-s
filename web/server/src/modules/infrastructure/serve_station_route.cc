#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "range/v3/view/transform.hpp"

#include "cereal/cereal.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "utl/logging.h"

#include "soro/utls/parse_int.h"

#include "soro/infrastructure/station/station_route.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_station_route(
    net::query_router::route_request const& req) const {
  using namespace soro::infra;

  auto const ctx = get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  station_route::id const sr_id{
      utls::parse_int<station_route::id::value_t>(req.path_params_[1])};

  if (sr_id >= (*ctx)->infra_->station_routes_.size()) {
    uLOG(utl::warn) << "Requesting station route " << sr_id
                    << " but there are only "
                    << (*ctx)->infra_->station_routes_.size()
                    << " station routes";
    return net::not_found_response(req);
  }

  auto const& sr = (*ctx)->infra_->station_routes_[sr_id];

  json_archive archive;
  archive.add()(
      cereal::make_nvp("id", sr->id_), cereal::make_nvp("name", sr->name_),
      cereal::make_nvp("ds100", sr->station_->ds100_),
      cereal::make_nvp(
          "path", sr->nodes() | ranges::views::transform([&](auto&& n) {
                    return (*ctx)->positions_.elements_[n->element_->get_id()];
                  })));
  return json_response(req, archive);
}

}  // namespace soro::server
