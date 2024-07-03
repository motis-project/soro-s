#include "soro/server/modules/infrastructure/positions.h"

#include <cstdint>
#include <algorithm>
#include <iterator>
#include <limits>
#include <tuple>
#include <utility>

#include "range/v3/algorithm/count_if.hpp"
#include "range/v3/algorithm/max.hpp"
#include "range/v3/view/transform.hpp"

#include "cista/containers/vector.h"

#include "utl/logging.h"
#include "utl/timer.h"
#include "utl/zip.h"

#include "soro/base/soro_types.h"

#include "soro/utls/coordinates/gps.h"
#include "soro/utls/coordinates/polar.h"
#include "soro/utls/graph/traversal.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/layout.h"
#include "soro/infrastructure/station/station_route.h"

namespace soro::server {

using namespace utl;

using namespace soro::infra;
using namespace soro::utls;

using rotation = double;

auto get_neighbours_with_coords(
    station::ptr const station,
    soro::vector_map<station::id, gps> const& station_coords) {
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

utls::gps interpolate_position(
    station::ptr const station,
    soro::vector_map<station::id, utls::gps> const& station_positions) {
  using namespace ranges;

  auto const neighbours_with_coords =
      get_neighbours_with_coords(station, station_positions);

  if (neighbours_with_coords.size() < 2) {
    return {};
  }

  utls::gps interpolated{0.0, 0.0};

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

    interpolated.lon_ += station_positions[neigh->id_].lon_ * d0;
    interpolated.lat_ += station_positions[neigh->id_].lat_ * d0;

    total_dist += d0;
  }

  interpolated.lon_ /= total_dist;
  interpolated.lat_ /= total_dist;

  return interpolated;
}

soro::vector_map<station::id, utls::gps> interpolate_station_positions(
    soro::vector_map<station::id, station::ptr> const& stations,
    soro::map<station::ds100, utls::gps> const& station_positions) {
  utl::scoped_timer const timer("interpolating missing station coordinates");

  soro::vector_map<station::id, utls::gps> interpolated(stations.size(),
                                                        gps::invalid());

  // transfer the already known gps positions into the interpolated set
  // as a seed
  for (auto const& station : stations) {
    auto const it = station_positions.find(station->ds100_);
    if (it == std::end(station_positions)) {
      continue;
    }

    interpolated[station->id_] = it->second;
  }

  for (bool work_to_do = true; work_to_do;) {
    work_to_do = false;

    for (auto const [station, pos] : utl::zip(stations, interpolated)) {
      if (pos.valid()) {
        continue;
      }

      pos = interpolate_position(station, interpolated);

      work_to_do = pos.valid();
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
        get_neighbours_with_coords(station, interpolated);
    if (neighbours_with_coords.empty()) {
      coords = {8.0, 47.0};
    } else {
      auto const& [neighbour, depth] = neighbours_with_coords.front();
      auto const neighbour_coords = interpolated[neighbour->id_];
      coords = {neighbour_coords.lon_ - depth * 0.1, neighbour_coords.lat_};
    }
  }

  // ensure every station has a position
  utls::ensure(interpolated.size() == stations.size());
  utls::ensure(
      utls::all_of(interpolated, [](auto&& gps) { return gps.valid(); }));

  return interpolated;
}

gps rotate_point_around_by(gps const& point, gps const& anchor,
                           rotation const by) {

  auto const distance = anchor.distance(point);
  auto const bearing = anchor.bearing(point);

  return anchor.move(distance, bearing + by);
}

void rotate_elements(
    soro::vector_map<station::id, station::ptr> const& stations,
    soro::vector_map<station::id, utls::gps> const& station_positions,
    soro::vector_map<station::id, rotation> const& station_rotation,
    soro::vector_map<element::id, utls::gps>& element_positions) {
  uLOG(utl::info) << "rotating elements";

  for (auto const& station : stations) {
    auto const station_center = station_positions[station->id_];
    auto const rotation = station_rotation[station->id_];

    for (auto const& element : station->elements_) {
      element_positions[element->get_id()] = rotate_point_around_by(
          element_positions[element->get_id()], station_center, rotation);
    }
  }
}

void rotate_nodes(
    soro::vector_map<station::id, station::ptr> const& stations,
    soro::vector_map<station::id, utls::gps> const& station_positions,
    soro::vector_map<station::id, rotation> const& station_rotation,
    soro::vector_map<node::id, utls::gps>& node_positions) {
  uLOG(utl::info) << "rotating nodes";

  for (auto const& station : stations) {
    auto const station_center = station_positions[station->id_];
    auto const rotation = station_rotation[station->id_];

    for (auto const& element : station->elements_) {
      for (auto const& node : element->nodes()) {
        node_positions[node->id_] = rotate_point_around_by(
            node_positions[node->id_], station_center, rotation);
      }
    }
  }
}

rotation get_best_rotation(
    station::ptr const station,
    soro::vector_map<station::id, gps> const& station_coords,
    soro::vector_map<element::id, gps> const& element_coords,
    soro::vector_map<station::id, rotation> const& station_rotations) {

  rotation best_rotation = std::numeric_limits<rotation>::max();
  double best_rotation_score = std::numeric_limits<double>::max();

  for (uint32_t degree = 0; degree < 360; degree += 5) {
    // rotation score is the total distance between all neighbouring borders
    // smaller score is better
    auto rotation_score = 0.0;
    for (auto const& border : station->borders_) {
      auto from = element_coords[border.element_->get_id()];
      auto const from_rotated_border = rotate_point_around_by(
          from, station_coords[station->id_], static_cast<rotation>(degree));

      auto to = element_coords[border.neighbour_element_->get_id()];
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

soro::vector_map<station::id, rotation> get_best_rotations(
    soro::vector_map<station::id, station::ptr> const& stations,
    soro::vector_map<station::id, utls::gps> const& station_positions,
    soro::vector_map<element::id, utls::gps> const& element_positions) {
  uLOG(info) << "calculating best rotation for every stations";

  std::ignore = station_positions;
  std::ignore = element_positions;

  soro::vector_map<station::id, rotation> station_rotations(stations.size(),
                                                            rotation{90.0});

  // we are only optimizing rotations for a single station at a time
  // do a couple of passes to improve the result
  for (auto pass = 0; pass < 3; ++pass) {
    for (auto const& station : stations) {
      station_rotations[station->id_] = get_best_rotation(
          station, station_positions, element_positions, station_rotations);
    }
  }

  return station_rotations;
}

auto resolve_overlaps(
    soro::vector<station::ptr> const& stations,
    soro::vector_map<station::id, gps> const& station_coords) {
  uLOG(info) << "[ Layout ] Resolving overlaps.";

  auto moved = station_coords;

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

coordinates get_station_center(
    station::ptr const s,
    soro::vector_map<element::id, coordinates> const& element_coordinates) {
  auto result = coordinates::min();

  for (auto const& e : s->elements_) {
    result.x_ = std::max(result.x_, element_coordinates[e->get_id()].x_);
    result.y_ = std::max(result.y_, element_coordinates[e->get_id()].y_);
  }

  result.x_ /= 2;
  result.y_ /= 2;

  return result;
};

soro::vector_map<element::id, utls::gps> get_element_positions(
    soro::vector_map<station::id, station::ptr> const& stations,
    soro::vector_map<station::id, utls::gps> const& station_positions,
    soro::vector_map<element::id, coordinates> const& element_coordinates) {
  utl::scoped_timer const timer("creating element positions");

  soro::vector_map<element::id, gps> element_positions(
      element_coordinates.size(), gps::invalid());

  for (auto const& station : stations) {
    auto const center_coordinate =
        get_station_center(station, element_coordinates);

    for (auto const& element : station->elements_) {
      auto shift = element_coordinates[element->get_id()] - center_coordinate;

      auto const LAYOUT_TO_GPS = 10.0;

      auto const x_shift = shift.x_ * LAYOUT_TO_GPS;
      auto const y_shift = shift.y_ * LAYOUT_TO_GPS;

      polar const p(x_shift, y_shift);

      element_positions[element->get_id()] =
          station_positions[station->id_].move(p.dist_, p.bearing_);
    }
  }

  return element_positions;
}

coordinates get_station_center(
    station::ptr const s,
    soro::vector_map<node::id, coordinates> const& node_coordinates) {
  auto result = coordinates::min();

  for (auto const& e : s->elements_) {
    for (auto const& n : e->nodes()) {
      result.x_ = std::max(result.x_, node_coordinates[n->id_].x_);
      result.y_ = std::max(result.y_, node_coordinates[n->id_].y_);
    }
  }

  result.x_ /= 2;
  result.y_ /= 2;

  return result;
};

soro::vector_map<node::id, utls::gps> get_node_positions(
    soro::vector_map<station::id, station::ptr> const& stations,
    soro::vector_map<station::id, utls::gps> const& station_positions,
    soro::vector_map<node::id, coordinates> const& node_coordinates) {
  utl::scoped_timer const timer("creating element positions");

  soro::vector_map<node::id, gps> node_positions(node_coordinates.size(),
                                                 gps::invalid());

  for (auto const& station : stations) {
    auto const center_coordinate =
        get_station_center(station, node_coordinates);

    for (auto const& element : station->elements_) {
      for (auto const& node : element->nodes()) {

        auto shift = node_coordinates[node->id_] - center_coordinate;

        auto const LAYOUT_TO_GPS = 10.0;

        auto const x_shift = shift.x_ * LAYOUT_TO_GPS;
        auto const y_shift = shift.y_ * LAYOUT_TO_GPS;

        polar const p(x_shift, y_shift);

        node_positions[node->id_] =
            station_positions[station->id_].move(p.dist_, p.bearing_);
      }
    }
  }

  return node_positions;
}

positions get_positions(
    infrastructure const& infra,
    soro::map<infra::station::ds100, utls::gps> const& station_pos) {
  utl::scoped_timer const timer("creating positions");

  positions result;

  // interpolate the missing station positions
  result.stations_ =
      interpolate_station_positions(infra->stations_, station_pos);

  // when we have a position for every station we can determine the element
  // positions
  result.elements_ = get_element_positions(infra->stations_, result.stations_,
                                           infra->layout_.element_coordinates_);
  // and the node positions
  result.nodes_ = get_node_positions(infra->stations_, result.stations_,
                                     infra->layout_.node_coordinates_);

  // as every station is currently aligned horizontally ..
  // .. determine the best rotation for aesthetics
  auto const rotations =
      get_best_rotations(infra->stations_, result.stations_, result.elements_);

  // actually rotate the elements
  rotate_elements(infra->stations_, result.stations_, rotations,
                  result.elements_);
  // and the nodes
  rotate_nodes(infra->stations_, result.stations_, rotations, result.nodes_);

  // ensure every element has a position
  utls::ensure(result.elements_.size() == infra->graph_.elements_.size());
  utls::ensure(
      utls::all_of(result.elements_, [](auto&& gps) { return gps.valid(); }));

  // ensure every node has a position
  utls::ensure(result.nodes_.size() == infra->graph_.nodes_.size());
  utls::ensure(
      utls::all_of(result.nodes_, [](auto&& gps) { return gps.valid(); }));

  return result;
}

}  // namespace soro::server
