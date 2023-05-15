#include "soro/infrastructure/layout/layout_to_gps.h"

#include "range/v3/algorithm/count_if.hpp"
#include "range/v3/algorithm/max.hpp"
#include "range/v3/view/transform.hpp"

#include <unordered_map>

#include "utl/logging.h"
#include "utl/zip.h"

#include "soro/utls/coordinates/polar.h"
#include "soro/utls/graph/traversal.h"

namespace soro::layout {

using namespace utl;

using namespace soro::infra;
using namespace soro::utls;

using rotation = double;

auto get_neighbours_with_coords(station::ptr const station,
                                soro::vector<gps> const& station_coords) {
  // gather all neighbours (any degree) with coordinates and their distances
  soro::vector<std::pair<station::ptr, uint32_t>> neighbours_with_coords;
  soro::map<station::ptr, uint32_t> dists;

  auto const handle_node = [&](auto&& s, auto&& prev_s) {
    dists[s] = (s == station ? 0 : dists[prev_s] + 1);

    if (station_coords[s->id_].valid()) {
      neighbours_with_coords.emplace_back(std::pair{s, dists[s]});
    }

    // we don't want to terminate when only a single neighbour with coords has
    // been found. termination handled by the get_neighbours lambda
    return false;
  };

  auto const get_neighbours = [&](station::ptr s) {
    // when the station has coords we don't want to look any further
    return station_coords[s->id_].valid() ? soro::vector<station::ptr>{}
                                          : s->neighbours();
  };

  utls::bfs(station, get_neighbours, handle_node);

  return neighbours_with_coords;
}

gps interpolate_coordinates(station::ptr station,
                            soro::vector<gps> const& station_coords) {
  using namespace ranges;

  auto const neighbours_with_coords =
      get_neighbours_with_coords(station, station_coords);

  if (neighbours_with_coords.size() < 2) {
    return {};
  }

  gps interpolated{0.0, 0.0};

  auto const max_dist =
      max(neighbours_with_coords |
          views::transform([](auto&& pair) { return pair.second; })) +
      1;

  gps::precision total_dist{0.0};

  for (auto const& [neigh, dist] : neighbours_with_coords) {
    auto d0 = max_dist - dist;
    // how strong is the station coupled with the neighbour
    // i.e. how many borders do they share
    auto const borders = station->borders_ | views::transform([](auto&& b) {
                           return b.neighbour_;
                         });
    auto const is_neigh = [&](auto&& n) { return n == neigh; };
    auto const coupling = count_if(borders, is_neigh);

    // TODO(julian) maybe use the coupling as a factor here
    if (dist == 1 && coupling > 4) {
      d0 *= 2;
    }

    interpolated.lon_ += station_coords[neigh->id_].lon_ * d0;
    interpolated.lat_ += station_coords[neigh->id_].lat_ * d0;

    total_dist += d0;
  }

  interpolated.lon_ /= total_dist;
  interpolated.lat_ /= total_dist;

  return interpolated;
}

auto interpolate_coordinates(soro::vector<station::ptr> const& stations,
                             soro::vector<gps> const& station_coords) {
  uLOG(info) << "[ Layout ] Interpolating missing station coordinates.";

  soro::vector<gps> interpolated = station_coords;

  for (bool work_to_do = true; work_to_do;) {
    work_to_do = false;

    for (auto const [station, coords] : utl::zip(stations, interpolated)) {
      if (coords.valid()) {
        continue;
      }

      coords = interpolate_coordinates(station, station_coords);

      work_to_do = coords.valid();
    }
  }

  // for all stations with only a single or no neighbour set up positions
  // just put stations with no neighbours somewhere
  // and stations with a single neighbour left to its neighbour
  for (auto const [station, coords] : utl::zip(stations, interpolated)) {
    if (coords.valid()) {
      continue;
    }

    auto const neighbours_with_coords =
        get_neighbours_with_coords(station, station_coords);
    if (neighbours_with_coords.empty()) {
      coords = {8.0, 47.0};
    } else if (neighbours_with_coords.size() == 1) {
      auto const& [neighbour, depth] = neighbours_with_coords.front();
      auto const neighbour_coords = station_coords[neighbour->id_];
      coords = {neighbour_coords.lon_ - depth * 0.1, neighbour_coords.lat_};
    }
  }

  return interpolated;
}

gps rotate_point_around_by(gps const& point, gps const& anchor,
                           rotation const by) {

  auto const distance = anchor.distance(point);
  auto const bearing = anchor.bearing(point);

  return anchor.move(distance, bearing + by);
}

auto rotate_elements(soro::vector<station::ptr> const& stations,
                     soro::vector<gps> const& station_coords,
                     soro::vector<rotation> const& station_rotation,
                     soro::vector<gps> const& element_coords) {
  uLOG(utl::info) << "[ Layout ] Rotating elements.";

  soro::vector<gps> rotated(element_coords);

  for (auto const& station : stations) {
    auto const station_center = station_coords[station->id_];
    auto const rotation = station_rotation[station->id_];

    for (auto const& element : station->elements_) {
      rotated[element->id()] = rotate_point_around_by(
          element_coords[element->id()], station_center, rotation);
    }
  }

  return rotated;
}

auto get_best_rotation(station::ptr station,
                       soro::vector<gps> const& station_coords,
                       soro::vector<gps> const& element_coords,
                       soro::vector<rotation> const& station_rotations) {
  rotation best_rotation = std::numeric_limits<rotation>::max();
  double best_rotation_score = std::numeric_limits<double>::max();

  for (uint32_t degree = 0; degree < 360; degree += 5) {
    // rotation score is the total distance between all neighbouring borders
    // smaller score is better
    auto rotation_score = 0.0;
    for (auto const& border : station->borders_) {
      auto from = element_coords[border.element_->id()];
      auto const from_rotated_border = rotate_point_around_by(
          from, station_coords[station->id_], static_cast<rotation>(degree));

      auto to = element_coords[border.neighbour_element_->id()];
      auto const to_rotated_border =
          rotate_point_around_by(to, station_coords[border.neighbour_->id_],
                                 station_rotations[border.neighbour_->id_]);

      rotation_score += from_rotated_border.distance(to_rotated_border);
    }

    if (rotation_score < best_rotation_score) {
      best_rotation_score = rotation_score;
      best_rotation = degree;
    }
  }

  return best_rotation;
}

auto get_best_rotations(soro::vector<station::ptr> const& stations,
                        soro::vector<gps> const& station_coords,
                        soro::vector<gps> const& element_coords) {
  uLOG(info) << "[ Layout ] Calculating best rotation of stations.";

  soro::vector<rotation> station_rotations(stations.size());
  std::fill(std::begin(station_rotations), std::end(station_rotations),
            rotation{0.0});

  // we are only optimizing rotations for a single station at a time
  // do a couple of passes to improve the result
  for (auto pass = 0; pass < 3; ++pass) {
    station_rotations = soro::to_vec(stations, [&](auto&& s) {
      return get_best_rotation(s, station_coords, element_coords,
                               station_rotations);
    });
  }

  return station_rotations;
}

auto resolve_overlaps(soro::vector<station::ptr> const& stations,
                      soro::vector<gps> const& station_coords) {
  uLOG(info) << "[ Layout ] Resolving overlaps.";

  soro::vector<gps> moved = station_coords;

  for (int i = 0; i < 4; ++i) {
    for (auto const& s1 : stations) {
      for (auto const& s2 : stations) {
        if (s1 == s2) {
          continue;
        }

        auto const& gps1 = station_coords[s1->id_];
        auto const& gps2 = station_coords[s2->id_];

        polar const p(gps1.lon_ - gps2.lon_, gps1.lat_ - gps2.lat_);
        if (p.dist_ < 0.002 && p.dist_ != 0.0) {
          [[maybe_unused]] auto const moved1 = gps1.move(400, p.bearing_);
          [[maybe_unused]] auto const moved2 = gps2.move(100, -p.bearing_);
        }
      }
    }
  }

  return moved;
}

auto get_element_gps_coords(soro::vector<station::ptr> const& stations,
                            soro::vector<gps> const& station_coords,
                            layout const& layout) {
  soro::vector<gps> element_gps_coords(layout.size());

  auto const get_station_center = [](auto const& l, station::ptr s) {
    layout::value_type result{0.0, 0.0};

    for (auto const& e : s->elements_) {
      result.x_ = std::max(result.x_, l[e->id()].x_);
      result.y_ = std::max(result.y_, l[e->id()].y_);
    }

    result.x_ /= 2;
    result.y_ /= 2;

    return result;
  };

  for (auto const& station : stations) {
    auto const station_center = get_station_center(layout, station);

    for (auto const& element : station->elements_) {
      auto x_shift = layout[element->id()].x_ - station_center.x_;
      auto y_shift = layout[element->id()].y_ - station_center.y_;

      auto const LAYOUT_TO_GPS = 10.0;

      x_shift *= LAYOUT_TO_GPS;
      y_shift *= LAYOUT_TO_GPS;

      polar const p(x_shift, y_shift);

      element_gps_coords[element->id()] =
          station_coords[station->id_].move(p.dist_, p.bearing_);
    }
  }

  return element_gps_coords;
}

std::pair<soro::vector<gps>, soro::vector<gps>> layout_to_gps(
    layout const& layout, soro::vector<station::ptr> const& stations,
    soro::vector<gps> const& station_coords) {
  soro::vector<gps> adjusted_station_coords;

  adjusted_station_coords = interpolate_coordinates(stations, station_coords);
  // TODO(julian) enable this
  //  adjusted_station_coords = resolve_overlaps(stations,
  //  adjusted_station_coords);

  auto element_coords =
      get_element_gps_coords(stations, adjusted_station_coords, layout);

  auto const rotations =
      get_best_rotations(stations, adjusted_station_coords, element_coords);

  auto rotated = rotate_elements(stations, adjusted_station_coords, rotations,
                                 element_coords);

  return {adjusted_station_coords, rotated};
}

}  // namespace soro::layout
