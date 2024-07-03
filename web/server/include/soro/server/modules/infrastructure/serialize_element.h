#pragma once

#include "utl/overloaded.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

inline void add_data_to_cereal_archive(json_archive& ar,
                                       infra::element_data const& ed) {
  ed.apply(utl::overloaded{
      [&](infra::eotd const& eotd) {
        ar.add()(cereal::make_nvp("signal", eotd.signal_));
      },
      [&](infra::slope const& slope) {
        ar.add()(cereal::make_nvp("slope", si::as_radian(slope)));
      },
      [&](infra::halt const& halt) {
        ar.add()(cereal::make_nvp("name", halt.name_));
        ar.add()(cereal::make_nvp("passenger", halt.is_passenger_));
        ar.add()(cereal::make_nvp("id_extern", halt.identifier_extern_));
        ar.add()(cereal::make_nvp("id_op", halt.identifier_operational_));
      },
      [&](infra::speed_limit const& limit) {
        ar.add()(cereal::make_nvp("limit", si::as_km_h(limit.limit_)));
        ar.add()(cereal::make_nvp("point_of_activation", limit.poa_));
      },
      [&](infra::main_signal const& ms) {
        ar.add()(cereal::make_nvp("name", ms.name_));
        ar.add()(cereal::make_nvp("type", ms.type_));
        ar.add()(cereal::make_nvp("skip approach", ms.skip_approach_));
      },
      [&](infra::switch_data const& switch_data) {
        ar.add()(cereal::make_nvp("name", switch_data.name_));
      }});
}

inline void serialize_element(json_archive& archive,
                              infra::element::ptr const e,
                              infra::infrastructure const& infra) {
  archive.add()(cereal::make_nvp("elementId", to_idx(e->get_id())));
  archive.add()(cereal::make_nvp("type", e->get_type_str()));

  auto const rp_id = infra->graph_.element_to_rp_id_[e->get_id()];
  archive.add()(cereal::make_nvp("issId", rp_id.value_or(rp_id.INVALID)));

  if (e->is_track_element()) {
    auto const& track = e->as<infra::track_element>();
    archive.add()(cereal::make_nvp("kilometrage", si::as_m(track.km())));
    archive.add()(cereal::make_nvp("direction", to_string(track.dir())));
    archive.add()(cereal::make_nvp("line", track.get_line()));
  }

  add_data_to_cereal_archive(archive, infra->graph_.element_data_[e->get_id()]);
}

}  // namespace soro::server
