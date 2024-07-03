#include "soro/server/modules/tiles/osm_export/interpolation.h"

#include <cmath>
#include <vector>

#include "soro/base/fp_precision.h"
#include "soro/base/soro_types.h"

#include "soro/utls/coordinates/angle.h"
#include "soro/utls/coordinates/cartesian.h"
#include "soro/utls/coordinates/coordinates.h"
#include "soro/utls/coordinates/gps.h"

#include "soro/infrastructure/graph/element.h"

namespace soro::server::osm_export {

using namespace soro::si;
using namespace soro::utls;
using namespace soro::infra;

gps get_bezier_curve(gps const p1, gps const p2, gps const p3, gps const p4,
                     double const t) {
  auto const t1 = p1.to_cartesian();
  auto const t2 = p2.to_cartesian();
  auto const t3 = p3.to_cartesian();
  auto const t4 = p4.to_cartesian();

  cartesian c;

  c.x_ = std::pow((1 - t), 3) * t1.x_ + 3 * std::pow((1 - t), 2) * t * (t2.x_) +
         3 * std::pow((1 - t), 1) * std::pow(t, 2) * (t3.x_) +
         std::pow(t, 3) * (t4.x_);
  c.y_ = std::pow((1 - t), 3) * (t1.y_) +
         3 * std::pow((1 - t), 2) * t * (t2.y_) +
         3 * std::pow((1 - t), 1) * std::pow(t, 2) * (t3.y_) +
         std::pow(t, 3) * (t4.y_);
  c.z_ = std::pow((1 - t), 3) * (t1.z_) +
         3 * std::pow((1 - t), 2) * t * (t2.z_) +
         3 * std::pow((1 - t), 1) * std::pow(t, 2) * (t3.z_) +
         std::pow(t, 3) * (t4.z_);

  return c.to_gps();
}

utls::gps get_control_point(
    element::ptr const from, element::ptr const to,
    soro::vector_map<element::id, gps> const& element_coords) {

  auto const from_gps = element_coords[from->get_id()];
  auto const to_gps = element_coords[to->get_id()];

  auto const bearing = from_gps.bearing(to_gps);
  auto const distance = from_gps.distance(to_gps);

  return from_gps.move(distance / 2.0, bearing)
      .move(distance / 10.0, bearing + 90.0);
}

utls::gps get_auxiliary_point(
    element::ptr const e1, element::ptr const e2,
    soro::vector_map<element::id, gps> const& element_coords) {

  element::ptr neighbour = nullptr;
  for (auto node : e1->neighbours()) {
    if (node == nullptr || node->get_id() == e2->get_id()) {
      continue;
    } else {
      neighbour = node;
    }
  }

  auto const e1_gps = element_coords[e1->get_id()];
  auto const e2_gps = element_coords[e2->get_id()];

  gps auxiliary_point = e1_gps;
  if (neighbour != nullptr) {
    auto const neighbour_gps = element_coords[neighbour->get_id()];

    double dist = sin(to_rad(e1_gps.lat_)) * sin(to_rad(e2_gps.lat_)) +
                  cos(to_rad(e1_gps.lat_)) * cos(to_rad(e2_gps.lat_)) *
                      cos(to_rad(e1_gps.lon_ - e2_gps.lon_));
    dist = acos(dist);
    dist = (EARTH_RADIUS / 1000.0 * PI * dist) / 180.0F;
    double units = 0.2 * dist;

    if (equal((e1_gps.lon_ - neighbour_gps.lon_), 0.0)) {
      units = units / 1000000.0;
      if (e1_gps.lat_ - neighbour_gps.lat_ > 0) {
        auxiliary_point.lat_ = e1_gps.lat_ + units;
      } else
        auxiliary_point.lat_ = e1_gps.lat_ - units;
    } else {
      double const gradient = (e1_gps.lat_ - neighbour_gps.lat_) /
                              (e1_gps.lon_ - neighbour_gps.lon_);

      if ((-1 < gradient) && (gradient < 1)) {
        units = units / 1.0;
      } else if ((-10 < gradient) && (gradient < 10)) {
        units = units / 10.0;
      } else if ((-100 < gradient) && (gradient < 100)) {
        units = units / 100.0;
      } else if ((-1000 < gradient) && (gradient < 1000)) {
        units = units / 1000.0;
      } else if ((-10000 < gradient) && (gradient < 10000)) {
        units = units / 10000.0;
      } else if ((-100000 < gradient) && (gradient < 100000)) {
        units = units / 100000.0;
      } else if ((-1000000 < gradient) && (gradient < 1000000)) {
        units = units / 1000000.0;
      } else {
        units = units / 100000000.0;
      }

      if (((e1_gps.lon_ - neighbour_gps.lon_) < 0)) {
        auxiliary_point.lat_ = e1_gps.lat_ - gradient * units;
        auxiliary_point.lon_ = e1_gps.lon_ - units;
      } else if (((e1_gps.lon_ - neighbour_gps.lon_) > 0)) {
        auxiliary_point.lat_ = e1_gps.lat_ + gradient * units;
        auxiliary_point.lon_ = e1_gps.lon_ + units;
      }
    }
  }

  return auxiliary_point;
}

interpolation compute_interpolation(
    element::ptr const e1, element::ptr const e2,
    soro::vector_map<element::id, utls::gps> const& element_positions) {
  interpolation interpol;

  interpol.first_elem_ = e1->get_id();
  interpol.second_elem_ = e2->get_id();

  gps const auxiliary_point1 = get_auxiliary_point(e1, e2, element_positions);
  gps const auxiliary_point2 = get_auxiliary_point(e2, e1, element_positions);

  for (auto counter = 1; counter <= 100; counter += 2) {
    double const t = counter / 100.0;

    gps const bezier_curve_elem =
        get_bezier_curve(element_positions[e1->get_id()], auxiliary_point1,
                         auxiliary_point2, element_positions[e2->get_id()], t);

    interpol.points_.push_back(bezier_curve_elem);
  }

  return interpol;
}

std::vector<utls::gps> bezier_curve::get_curve(
    soro::size_t const points) const {
  std::vector<utls::gps> result;
  result.reserve(points);

  auto const p0 = from_.to_cartesian();
  auto const p1 = control_.to_cartesian();
  auto const p2 = to_.to_cartesian();

  for (soro::size_t i = 0; i < points; ++i) {
    auto const t = static_cast<double>(i) / static_cast<double>(points - 1);

    auto const c =
        std::pow((1 - t), 2) * p0 + 2 * (1 - t) * t * p1 + std::pow(t, 2) * p2;

    result.emplace_back(c.to_gps());
  }

  return result;
}

bezier_curve get_quadratic_bezier_curve(
    element::ptr const e1, element::ptr const e2,
    soro::vector_map<element::id, utls::gps> const& element_positions) {
  return {.from_ = element_positions[e1->get_id()],
          .control_ = get_control_point(e1, e2, element_positions),
          .to_ = element_positions[e2->get_id()]};
}

}  // namespace soro::server::osm_export
