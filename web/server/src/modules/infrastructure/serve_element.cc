#include "soro/server/modules/infrastructure/infrastructure.h"

#include "net/web_server/responses.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"
#include "soro/utls/parse_int.h"

namespace soro::server {

using namespace soro::infra;

template <typename Archive>
void add_data_to_cereal_archive(Archive& archive, element_data_t const& ed) {
  ed.apply(utl::overloaded{
      [&](eotd const& eotd) {
        archive(cereal::make_nvp("signal", eotd.signal_));
      },
      [&](slope const& slope) {
        archive(cereal::make_nvp("rising", si::as_precision(slope.rising_)));
        archive(cereal::make_nvp("falling", si::as_precision(slope.falling_)));
      },
      [&](halt const& halt) {
        archive(cereal::make_nvp("name", halt.name_));
        archive(cereal::make_nvp("passenger", halt.is_passenger_));
        archive(cereal::make_nvp("id_extern", halt.identifier_extern_));
        archive(cereal::make_nvp("id_op", halt.identifier_operational_));
      },
      [&](speed_limit const& limit) {
        archive(cereal::make_nvp("limit", si::as_km_h(limit.limit_)));
        archive(cereal::make_nvp("point_of_activation", limit.poa_));
      },
      [&](main_signal const& ms) {
        archive(cereal::make_nvp("name", ms.name_));
      },
      [&](switch_data const& switch_data) {
        archive(cereal::make_nvp("name", switch_data.name_));
      }});
}

net::web_server::string_res_t infra_state::serve_element(
    net::query_router::route_request const& req) const {
  auto const infra = get_infra(req.path_params_.front());
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  auto const e_id = utls::parse_int<element_id>(req.path_params_[1]);
  auto const& element = (**infra)->graph_.elements_[e_id];

  json_archive archive;

  archive.add()(cereal::make_nvp("id", e_id));

  if (element->is_directed_track_element()) {
    auto const& track = element->as<track_element>();
    archive.add()(cereal::make_nvp("kilometrage", si::as_m(track.km_)));
    archive.add()(cereal::make_nvp("rising", track.rising_));
    archive.add()(cereal::make_nvp("line", track.line_));
  }

  add_data_to_cereal_archive(archive.add(),
                             (**infra)->graph_.element_data_[e_id]);

  return json_response(req, archive);
}

}  // namespace soro::server
