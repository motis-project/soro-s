#pragma once

#include "pugixml.hpp"

#include "soro/infrastructure/base_infrastructure.h"
#include "soro/infrastructure/parsers/iss/iss_types.h"
#include "soro/server/osm_export/interpolation.h"

namespace soro::server::osm_export {

namespace infra = soro::infra;
namespace fs = std::filesystem;

/**
 * elem_ the element
 * station_ the station the element belongs to
 */
struct osm_element {
  infra::element_ptr elem_;
  infra::station::ptr station_;
};

struct osm_information {

  // all nodes in the xml
  // size_t id
  std::vector<
      std::pair<size_t, soro::variant<infra::station::ptr, osm_element>>>
      nodes_;

  // the ways in the xml, between two elements in nodes_
  std::vector<std::pair<infra::element_id, infra::element_id>> ways_;

  // the interpolations
  std::vector<interpolation> interpolations_;

  // the connection between nodes
  // prevents duplicate ways
  std::map<infra::element_id, std::vector<infra::element_id>> element_to_way_;

  // key: id of border element
  // value: id of border element in other station and xml id of interpolation
  // nodes
  // can be used for signal station routes
  std::map<infra::element_id, std::pair<infra::element_id, std::vector<size_t>>>
      element_to_interpolation_nodes_;
  // ids of elements in station route, id of station_route_way
  std::vector<std::pair<std::vector<size_t>, size_t>> station_route_nodes_;
};

pugi::xml_document export_to_osm(infra::base_infrastructure const& iss);
void export_and_write(infra::base_infrastructure const& iss,
                      fs::path const& out);

}  // namespace soro::server::osm_export

namespace soro::server::osm_export::detail {

void create_station_osm(pugi::xml_node& osm, infra::station::ptr station,
                        std::size_t const osm_id,
                        soro::vector<utls::gps> const& station_coords);

void create_element_osm(pugi::xml_node& osm, infra::element_ptr e,
                        soro::vector<utls::gps> const& element_coords);

void create_way_osm(pugi::xml_node& osm_node, infra::element_id const first,
                    infra::element_id const second, size_t const id);

}  // namespace soro::server::osm_export::detail
