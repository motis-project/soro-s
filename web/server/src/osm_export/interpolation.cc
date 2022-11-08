#include "soro/server/osm_export/interpolation.h"

#include "soro/utls/coordinates/angle.h"
#include "soro/utls/coordinates/cartesian.h"
#include "soro/utls/coordinates/gps.h"

#include "soro/infrastructure/graph/graph.h"

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

gps get_auxiliary_point(element_ptr e1, element_ptr e2,
                        soro::vector<gps> const& element_coords) {

  element_ptr neighbour = nullptr;
  for (auto node : e1->neighbours()) {
    if (node == nullptr || node->id() == e2->id()) {
      continue;
    } else {
      neighbour = node;
    }
  }

  auto const e1_gps = element_coords[e1->id()];
  auto const e2_gps = element_coords[e2->id()];

  gps auxiliary_point = e1_gps;
  if (neighbour != nullptr) {
    auto const neighbour_gps = element_coords[neighbour->id()];

    double dist = 0.0;
    dist = sin(to_rad(e1_gps.lat_)) * sin(to_rad(e2_gps.lat_)) +
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

interpolation compute_interpolation(element_ptr e1, element_ptr e2,
                                    soro::vector<gps> const& element_coords) {
  interpolation interpol;

  interpol.first_elem_ = e1->id();
  interpol.second_elem_ = e2->id();

  gps const auxiliary_point1 = get_auxiliary_point(e1, e2, element_coords);
  gps const auxiliary_point2 = get_auxiliary_point(e2, e1, element_coords);

  for (auto counter = 1; counter <= 100; counter += 2) {
    double const t = counter / 100.0;

    gps const bezier_curve_elem =
        get_bezier_curve(element_coords[e1->id()], auxiliary_point1,
                         auxiliary_point2, element_coords[e2->id()], t);

    interpol.points_.push_back(bezier_curve_elem);
  }

  return interpol;
}

}  // namespace soro::server::osm_export
