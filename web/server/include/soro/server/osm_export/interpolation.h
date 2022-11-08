#pragma once

#include "soro/utls/coordinates/gps.h"

#include "soro/infrastructure/graph/graph.h"

namespace soro::server::osm_export {

struct interpolation {
  infra::element_id first_elem_;
  std::vector<utls::gps> points_;
  infra::element_id second_elem_;
};

interpolation compute_interpolation(
    infra::element_ptr e1, infra::element_ptr e2,
    soro::vector<utls::gps> const& element_coords);

}  // namespace soro::server::osm_export
