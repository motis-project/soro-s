#include "soro/infrastructure/interlocking/get_interlocking.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/logging.h"
#include "utl/pairwise.h"
#include "utl/pipes.h"
#include "utl/timer.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/utls/map/insert_or.h"

#if defined(SORO_CUDA)
#include "soro/infrastructure/gpu/exclusion.h"
#endif

using namespace soro::utls;

namespace soro::infra {

auto get_halting_at(soro::vector<interlocking_route> const& interlocking_routes,
                    infrastructure const& infra) {
  utl::scoped_timer const t("Creating halt to interlocking routes mapping");

  soro::vector<interlocking_route::ids> halting_at(infra->graph_.nodes_.size());

  for (auto const& interlocking_route : interlocking_routes) {
    for (auto const& sub_path :
         interlocking_route.iterate_station_routes(*infra)) {
      auto const handle_halt_node = [&](auto&& halt_idx) {
        if (sub_path.contains(halt_idx)) {
          halting_at[sub_path.station_route_->nodes(halt_idx)->id_]
              .emplace_back(interlocking_route.id_);
        }
      };

      auto const passenger_halt =
          sub_path.station_route_->get_halt_idx(rs::FreightTrain::NO);
      if (passenger_halt.has_value()) {
        handle_halt_node(*passenger_halt);
      }

      auto const freight_halt =
          sub_path.station_route_->get_halt_idx(rs::FreightTrain::YES);
      if (freight_halt.has_value()) {
        handle_halt_node(*freight_halt);
      }
    }
  }

  for (auto& v : halting_at) {
    utls::sort(v);
  }

  return halting_at;
}

auto get_ending_at(soro::vector<interlocking_route> const& irs,
                   infrastructure const& infra) {
  utl::scoped_timer const timer("creating ending at");
  soro::vector<interlocking_route::ids> ending_at(infra->graph_.nodes_.size());

  for (auto const& interlocking_route : irs) {
    auto const last_node = interlocking_route.last_node(infra);

    ending_at[last_node->id_].emplace_back(interlocking_route.id_);
  }

  for (auto& v : ending_at) {
    utls::sort(v);
  }

  utls::ensures([&]() {
    auto const acc =
        utls::accumulate(ending_at, soro::size_t{0},
                         [](auto&& a, auto&& v) { return a + v.size(); });
    utls::ensure(
        acc == irs.size(),
        "Total interlocking routes {}, but accumulated starting at {}.",
        irs.size(), acc);
  });

  return ending_at;
}

auto get_starting_at(soro::vector<interlocking_route> const& irs,
                     infrastructure const& infra) {
  utl::scoped_timer const timer("creating starting at");

  soro::vector<interlocking_route::ids> starting_at(
      infra->graph_.nodes_.size());

  for (auto const& interlocking_route : irs) {
    auto const first_node = interlocking_route.first_node(infra);
    starting_at[first_node->id_].emplace_back(interlocking_route.id_);
  }

  for (auto& v : starting_at) {
    utls::sort(v);
  }

  utls::ensures([&]() {
    auto const acc =
        utls::accumulate(starting_at, soro::size_t{0},
                         [](auto&& a, auto&& v) { return a + v.size(); });
    utls::ensure(
        acc == irs.size(),
        "Total interlocking routes {}, but accumulated starting at {}.",
        irs.size(), acc);
  });

  return starting_at;
}

soro::vector<soro::vector<interlocking_route::id>> get_sr_to_participating_irs(
    soro::vector<interlocking_route> const& interlocking_routes,
    infrastructure const& infra) {
  utl::scoped_timer const participating_timer(
      "Creating station route to participating interlocking routes mapping");

  soro::vector<interlocking_route::ids> sr_to_participating_irs(
      infra->station_routes_.size());

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

soro::vector<interlocking_route> get_interlocking_routes_from_sr(
    station_route::ptr sr, station_route_graph const& srg, lines const& lines) {
  utls::expect(sr->can_start_an_interlocking(srg),
               "SR {} cannot start an interlocking route.", sr->id_);

  soro::vector<interlocking_route> routes;

  // reserve 2^16 elements as around there is the maximum of interlocking
  // routes a single station route produces
  routes.reserve(std::pow(2, 16));

  auto const fill_paths = [&srg, &routes, &lines](
                              station_route::ptr const& route,
                              interlocking_route ir,
                              auto const& get_paths_ref) {
    if (route->requires_etcs(lines)) {
      return;
    }

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
    infrastructure const& infra) {
  utl::scoped_timer const routes_timer("Generating Interlocking Routes");

  soro::vector<interlocking_route> interlocking_routes;
  interlocking_routes.reserve(infra->station_routes_.size() * 30);

  for (auto const& sr : infra->station_routes_) {
    // add all interlocking routes that start with this station route.
    // do not generate them when etcs is required to use the station route.
    if (sr->can_start_an_interlocking(infra->station_route_graph_) &&
        !sr->requires_etcs(infra->lines_)) {
      utl::concat(interlocking_routes,
                  get_interlocking_routes_from_sr(
                      sr, infra->station_route_graph_, infra->lines_));
    }

    if (sr->path_->main_signals_.size() > 1) {
      utl::concat(interlocking_routes, get_internal_interlocking_route(sr));
    }

    if (!sr->path_->main_signals_.empty() &&
        infra->station_route_graph_.predeccesors_[sr->id_].empty()) {
      interlocking_routes.emplace_back(get_leading_interlocking_route(sr));
    }

    if (!sr->path_->main_signals_.empty() &&
        infra->station_route_graph_.successors_[sr->id_].empty()) {
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
    infrastructure const& infra) {
  soro::vector<soro::vector<interlocking_route::id>> station_to_irs(
      infra->stations_.size());

  for (auto const& ir : interlocking_routes) {
    utl::all(ir.station_routes_) | utl::transform([&](auto&& sr) {
      return infra->station_routes_[sr]->station_;
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

interlocking get_interlocking(infrastructure_t const& infra_t) {
  infrastructure const infra(&infra_t);

  utl::scoped_timer const irs_timer("creating interlocking");

  interlocking irs;

  irs.routes_ = get_interlocking_routes(infra);

  irs.halting_at_ = get_halting_at(irs.routes_, infra);

  irs.starting_at_ = get_starting_at(irs.routes_, infra);
  irs.ending_at_ = get_ending_at(irs.routes_, infra);

  irs.sr_to_participating_irs_ =
      get_sr_to_participating_irs(irs.routes_, infra);
  irs.station_to_irs_ = get_station_to_interlocking_routes(irs.routes_, infra);

  print_interlocking_stats(irs.routes_);

  return irs;
}

}  // namespace soro::infra
