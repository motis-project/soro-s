#include "soro/infrastructure/interlocking/get_interlocking_subsystem.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/logging.h"
#include "utl/pairwise.h"
#include "utl/pipes.h"
#include "utl/timer.h"

#include "soro/utls/map/insert_or.h"

#include "soro/infrastructure/interlocking/exclusion.h"

#if defined(SORO_CUDA)
#include "soro/infrastructure/gpu/exclusion.h"
#endif

using namespace soro::utls;

namespace soro::infra {

auto get_halting_at(soro::vector<interlocking_route> const& interlocking_routes,
                    infrastructure_t const& infra) {
  utl::scoped_timer const t("Creating halt to interlocking routes mapping");

  soro::vector<soro::vector<interlocking_route::id>> halting_at(
      infra.graph_.nodes_.size());

  for (auto const& interlocking_route : interlocking_routes) {
    for (auto const& sub_path :
         interlocking_route.iterate_station_routes(infra)) {

      auto const handle_halt_node = [&](auto&& halt_idx) {
        if (sub_path.contains(halt_idx)) {
          halting_at[sub_path.station_route_->nodes(halt_idx)->id_]
              .emplace_back(interlocking_route.id_);
        }
      };

      sub_path.station_route_->get_halt_idx(rs::FreightTrain::NO)
          .execute_if(handle_halt_node);
      sub_path.station_route_->get_halt_idx(rs::FreightTrain::YES)
          .execute_if(handle_halt_node);
    }
  }

  for (auto& v : halting_at) {
    utls::sort(v);
  }

  return halting_at;
}

auto get_starting_at(
    soro::vector<interlocking_route> const& interlocking_routes,
    infrastructure_t const& infra) {
  soro::vector<soro::vector<interlocking_route::id>> starting_at(
      infra.graph_.nodes_.size());

  for (auto const& interlocking_route : interlocking_routes) {

    // TODO(julian) when we wont pass base_infrastructure, but instead
    // infrastructure we can simply use interlocking_route->first_node(infra)
    auto const& first_sr =
        *infra.station_routes_[interlocking_route.station_routes_.front()];
    auto const first_node = first_sr.nodes(interlocking_route.start_offset_);

    starting_at[first_node->id_].emplace_back(interlocking_route.id_);
  }

  for (auto& v : starting_at) {
    utls::sort(v);
  }

  utls::sasserts([&]() {
    auto const acc =
        utls::accumulate(starting_at, soro::size_t{0},
                         [](auto&& acc, auto&& v) { return acc + v.size(); });
    utls::sassert(
        acc == interlocking_routes.size(),
        "Total interlocking routes {}, but accumulated starting at {}.",
        interlocking_routes.size(), acc);
  });

  return starting_at;
}

soro::vector<soro::vector<interlocking_route::id>> get_sr_to_participating_irs(
    soro::vector<interlocking_route> const& interlocking_routes,
    infrastructure_t const& infra) {
  utl::scoped_timer const participating_timer(
      "Creating station route to participating interlocking routes mapping");

  soro::vector<soro::vector<interlocking_route::id>> sr_to_participating_irs(
      infra.station_routes_.size());

  for (auto const& interlocking_route : interlocking_routes) {
    for (auto const station_route_id : interlocking_route.station_routes_) {
      sr_to_participating_irs[station_route_id].emplace_back(
          interlocking_route.id_);
    }
  }

  for (auto& v : sr_to_participating_irs) {
    utls::sort(v);
  }

  return sr_to_participating_irs;
}

/*
 * Given a station route (not a signal one!) returns all internal signal
 * station routes. That is, all signal station routes that are completely
 * contained inside the station route.
 *
 * Number of internal signal station routes = Number of main signals in
 * station route - 1
 */
soro::vector<interlocking_route> get_internal_interlocking_route(
    station_route::ptr sr) {
  soro::vector<interlocking_route> interlocking_routes;

  for (auto [from, to] : utl::pairwise(sr->path_->main_signals_)) {
    interlocking_routes.emplace_back(
        interlocking_route{.id_ = interlocking_route::INVALID,
                           .start_offset_ = from,
                           .end_offset_ = static_cast<node::idx>(to + 1),
                           .station_routes_ = {sr->id_}});
  }

  return interlocking_routes;
}

interlocking_route get_leading_interlocking_route(station_route::ptr const sr) {
  return {.id_ = interlocking_route::INVALID,
          .start_offset_ = 0,
          .end_offset_ =
              static_cast<node::idx>(sr->path_->main_signals_.front() + 1),
          .station_routes_ = {sr->id_}};
}

interlocking_route get_trailing_interlocking_route(
    station_route::ptr const sr) {
  return {.id_ = interlocking_route::INVALID,
          .start_offset_ = sr->path_->main_signals_.back(),
          .end_offset_ = sr->size(),
          .station_routes_ = {sr->id_}};
}

std::string generate_dot_tree(station_route::ptr const route,
                              station_route_graph const& srg) {
  std::set<station_route::id> ids;

  auto const generate_graphviz = [&](station_route::ptr const sr,
                                     auto&& generate_ref) {
    ids.insert(sr->id_);

    if (sr->can_end_an_interlocking(srg) && sr->id_ != route->id_) {
      return;
    }

    for (auto const& neighbour : srg.successors_[sr->id_]) {
      generate_ref(neighbour, generate_ref);
    }
  };

  generate_graphviz(route, generate_graphviz);

  std::string dot;
  for (auto const& id : ids) {
    for (auto const& neigh : srg.successors_[id]) {
      dot += std::to_string(id) + " -> " + std::to_string(neigh->id_) + ";\n";
    }
  }

  return dot;
}

soro::vector<interlocking_route> get_interlocking_routes_from_sr(
    station_route::ptr sr, station_route_graph const& srg) {
  soro::vector<interlocking_route> routes;

  // reserve 2^16 elements as around there is the maximum of interlocking
  // routes a single station route produces
  routes.reserve(std::pow(2, 16));

  auto const fill_paths = [&srg, &routes](station_route::ptr const& route,
                                          interlocking_route ir,
                                          auto const& get_paths_ref) {
    ir.station_routes_.emplace_back(route->id_);

    if (route->can_end_an_interlocking(srg)) {
      ir.end_offset_ = route->path_->main_signals_.empty()
                           ? route->size()
                           : route->path_->main_signals_.front() + 1;
      routes.push_back(std::move(ir));
      return;
    }

    for (auto const& neighbour : srg.successors_[route->id_]) {
      get_paths_ref(neighbour, ir, get_paths_ref);
    }
  };

  interlocking_route const initial_ir{
      .id_ = interlocking_route::INVALID,
      .start_offset_ = sr->path_->main_signals_.empty()
                           ? node::idx{0}
                           : sr->path_->main_signals_.back(),
      .end_offset_ = static_cast<node::idx>(sr->size()),
      .station_routes_ = {sr->id_}};

  for (auto const& succ : srg.successors_[sr->id_]) {
    fill_paths(succ, initial_ir, fill_paths);
  }

  return routes;
}

soro::vector<interlocking_route> get_interlocking_routes(
    infrastructure_t const& infra) {
  utl::scoped_timer const routes_timer("Generating Interlocking Routes");

  soro::vector<interlocking_route> interlocking_routes;
  interlocking_routes.reserve(infra.station_routes_.size() * 30);

  for (auto const& sr : infra.station_routes_) {
    if (sr->can_start_an_interlocking(infra.station_route_graph_)) {
      utl::concat(interlocking_routes, get_interlocking_routes_from_sr(
                                           sr, infra.station_route_graph_));
    }

    if (sr->path_->main_signals_.size() > 1) {
      utl::concat(interlocking_routes, get_internal_interlocking_route(sr));
    }

    if (!sr->path_->main_signals_.empty() &&
        infra.station_route_graph_.predeccesors_[sr->id_].empty()) {
      interlocking_routes.emplace_back(get_leading_interlocking_route(sr));
    }

    if (!sr->path_->main_signals_.empty() &&
        infra.station_route_graph_.successors_[sr->id_].empty()) {
      interlocking_routes.emplace_back(get_trailing_interlocking_route(sr));
    }
  }

  interlocking_route::id current_id = 0;
  for (auto& ir : interlocking_routes) {
    ir.id_ = current_id++;
  }

  return interlocking_routes;
}

void print_interlocking_stats(soro::vector<interlocking_route> const& irs) {
  std::size_t length_in_srs = 0;

  for (auto const& ir : irs) {
    length_in_srs += ir.station_routes_.size();
  }

  uLOG(utl::info) << "Generated " << irs.size() << " interlocking routes.";
  uLOG(utl::info) << "Average length in station routes of an interlocking: "
                  << length_in_srs / irs.size();
}

auto get_station_to_interlocking_routes(
    soro::vector<interlocking_route> const& interlocking_routes,
    infrastructure_t const& infra) {
  soro::vector<soro::vector<interlocking_route::id>> station_to_irs(
      infra.stations_.size());

  for (auto const& ir : interlocking_routes) {
    utl::all(ir.station_routes_) | utl::transform([&](auto&& sr) {
      return infra.station_routes_[sr]->station_;
    }) | utl::for_each([&](auto&& station) {
      station_to_irs[station->id_].push_back(ir.id_);
    });
  }

  for (auto& station_irs : station_to_irs) {
    utl::erase_duplicates(station_irs);
    utls::sort(station_irs);
  }

  return station_to_irs;
}

interlocking_subsystem get_interlocking_subsystem(
    infrastructure_t const& infra, bool const determine_exclusions) {
  utl::scoped_timer const irs_timer("Creating Interlocking Subsystem");

  interlocking_subsystem irs;

  irs.routes_ = get_interlocking_routes(infra);

  irs.halting_at_ = get_halting_at(irs.routes_, infra);
  irs.starting_at_ = get_starting_at(irs.routes_, infra);
  irs.sr_to_participating_irs_ =
      get_sr_to_participating_irs(irs.routes_, infra);
  irs.station_to_irs_ = get_station_to_interlocking_routes(irs.routes_, infra);

  if (determine_exclusions) {
    irs.exclusions_ = get_interlocking_route_exclusions(irs, infra);
  }

  print_interlocking_stats(irs.routes_);

  return irs;
}

}  // namespace soro::infra
