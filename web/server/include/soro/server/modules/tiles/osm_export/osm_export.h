#pragma once

#include "pugixml.hpp"

#include "soro/infrastructure/infrastructure_t.h"

#include "soro/server/modules/infrastructure/positions.h"
#include "soro/server/modules/tiles/osm_export/interpolation.h"

namespace soro::server::osm_export {

pugi::xml_document export_to_osm(infra::infrastructure const& infra,
                                 positions const& positions);

void export_and_write(infra::infrastructure const& infra,
                      positions const& positions,
                      std::filesystem::path const& out);

}  // namespace soro::server::osm_export

namespace soro::server {

void export_and_write2(infra::infrastructure const& infra,
                       positions const& positions,
                       std::filesystem::path const& out);

}

namespace soro::server::osm_export::detail {

void create_station_osm(
    pugi::xml_node& osm, infra::station::ptr station, std::size_t const osm_id,
    soro::vector_map<infra::station::id, utls::gps> const& station_coords);

void create_element_osm(
    pugi::xml_node& osm, infra::element::ptr e,
    soro::vector_map<infra::element::id, utls::gps> const& element_coords);

void create_way_osm(pugi::xml_node& osm_node, infra::element::id const first,
                    infra::element::id const second, size_t const id);

}  // namespace soro::server::osm_export::detail
