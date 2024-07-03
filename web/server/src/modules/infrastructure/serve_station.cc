#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "cereal/cereal.hpp"
#include "cereal/details/helpers.hpp"
#include "cereal/types/vector.hpp"  // NOLINT

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "utl/logging.h"

#include "soro/base/soro_types.h"

#include "soro/utls/parse_int.h"

#include "soro/infrastructure/station/station.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT

#include "soro/server/cereal/json_archive.h"

namespace soro::infra {

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive,
                                    station_route::ptr const s) {
  auto const from = s->nodes().front()->element_->get_id();
  auto const to = s->nodes().back()->element_->get_id();

  archive(cereal::make_nvp("id", s->id_), cereal::make_nvp("name", s->name_),
          cereal::make_nvp("from_element", from),
          cereal::make_nvp("to_element", to));
}

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(
    Archive& archive,
    soro::map<soro::string, soro::infra::station_route::ptr> const&
        station_routes) {
  archive(cereal::make_size_tag(
      static_cast<cereal::size_type>(station_routes.size())));

  for (auto const& [_, station_route] : station_routes) {
    archive(station_route);
  }
}

}  // namespace soro::infra

namespace soro::server {

net::web_server::string_res_t infrastructure_module::serve_station(
    net::query_router::route_request const& req) const {
  using namespace soro::infra;

  auto const ctx = get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  station::id const s_id{
      utls::parse_int<station::id::value_t>(req.path_params_[1])};

  if (s_id >= (*ctx)->infra_->stations_.size()) {
    uLOG(utl::warn) << "requesting station with id " << s_id
                    << " but there are only "
                    << (*ctx)->infra_->stations_.size() << " stations";
    return net::not_found_response(req);
  }
  auto const& station = (*ctx)->infra_->stations_[s_id];
  auto const& irs = (*ctx)->infra_->interlocking_.station_to_irs_[s_id];

  json_archive archive;
  archive.add()(cereal::make_nvp("id", as_val(station->id_)),
                cereal::make_nvp("ds100", station->ds100_),
                cereal::make_nvp("stationRoutes", station->station_routes_),
                cereal::make_nvp("interlockingRouteIds", irs));
  return json_response(req, archive);
}

}  // namespace soro::server
