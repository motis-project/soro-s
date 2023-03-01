#include "soro/server/modules/infrastructure/infrastructure.h"

// #include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"

#include "net/web_server/responses.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"
#include "soro/utls/parse_int.h"

namespace soro::infra {

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive,
                                    station_route::ptr const s) {
  archive(cereal::make_nvp("id", s->id_), cereal::make_nvp("name", s->name_));
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

net::web_server::string_res_t infra_state::serve_station(
    net::query_router::route_request const& req) const {
  using namespace soro::infra;

  auto const infra = get_infra(req.path_params_.front());
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  auto const s_id = utls::parse_int<station::id>(req.path_params_[1]);

  if (s_id >= (**infra)->stations_.size()) {
    uLOG(utl::warn) << "requesting station with id " << s_id
                    << " but there are only " << (**infra)->stations_.size()
                    << " stations";
    return net::not_found_response(req);
  }
  auto const& station = (**infra)->stations_[s_id];
  auto const& irs = (**infra)->interlocking_.station_to_irs_[s_id];

  json_archive archive;
  archive.add()(cereal::make_nvp("id", station->id_),
                cereal::make_nvp("ds100", station->ds100_),
                cereal::make_nvp("station_routes", station->station_routes_),
                cereal::make_nvp("interlocking_routes", irs));
  return json_response(req, archive);
}

}  // namespace soro::server
