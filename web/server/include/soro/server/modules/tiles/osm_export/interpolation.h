#pragma once

#include "soro/utls/coordinates/gps.h"

#include "soro/infrastructure/graph/graph.h"

namespace soro::server::osm_export {

struct interpolation {
  infra::element::id first_elem_;
  std::vector<utls::gps> points_;
  infra::element::id second_elem_;
};

interpolation compute_interpolation(
    infra::element::ptr const e1, infra::element::ptr const e2,
    soro::vector_map<infra::element::id, utls::gps> const& element_positions);

struct bezier_curve {
  std::vector<utls::gps> get_curve(soro::size_t const points) const;

  utls::gps from_;
  utls::gps control_;
  utls::gps to_;
};

bezier_curve get_quadratic_bezier_curve(
    infra::element::ptr const e1, infra::element::ptr const e2,
    soro::vector_map<infra::element::id, utls::gps> const& element_positions);

}  // namespace soro::server::osm_export
