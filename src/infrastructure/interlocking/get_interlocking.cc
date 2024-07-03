#include "soro/infrastructure/interlocking/get_interlocking.h"

#include <cmath>
#include <cstddef>
#include <iterator>
#include <utility>

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/logging.h"
#include "utl/pairwise.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/is_sorted.h"
#include "soro/utls/std_wrapper/sort.h"
#include "soro/utls/std_wrapper/upper_bound.h"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/rolling_stock/stop_mode.h"

#if defined(SORO_CUDA)
#include "soro/infrastructure/gpu/exclusion.h"
#endif

using namespace soro::utls;

namespace soro::infra {

soro::vector_map<node::id, interlocking_route::ids> get_halting_at(
    soro::vector_map<ir_id, interlocking_route> const& interlocking_routes,
    infrastructure const& infra) {
  utl::scoped_timer const t("creating halt to interlocking routes mapping");

  soro::vector_map<node::id, interlocking_route::ids> halting_at(
      infra->graph_.nodes_.size());

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
          sub_path.station_route_->get_halt_idx(rs::stop_mode::passenger);
      if (passenger_halt.has_value()) {
        handle_halt_node(*passenger_halt);
      }

      auto const freight_halt =
          sub_path.station_route_->get_halt_idx(rs::stop_mode::freight);
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

soro::vector_map<node::id, interlocking_route::ids> get_ending_at(
    soro::vector_map<ir_id, interlocking_route> const& irs,
    infrastructure const& infra) {
  utl::scoped_timer const timer("creating ending at");

  soro::vector_map<node::id, interlocking_route::ids> ending_at(
      infra->graph_.nodes_.size());

  for (auto const& interlocking_route : irs) {
    auto const last_node = interlocking_route.last_node(infra);

    ending_at[last_node->id_].emplace_back(interlocking_route.id_);
  }

  for (auto& v : ending_at) {
    utls::sort(v);
  }

  utls::ensures([&] {
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

soro::vector_map<node::id, interlocking_route::ids> get_starting_at(
    soro::vector_map<ir_id, interlocking_route> const& irs,
    infrastructure const& infra) {
  utl::scoped_timer const timer("creating starting at");

  soro::vector_map<node::id, interlocking_route::ids> starting_at(
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

soro::vector_map<station_route::id, interlocking_route::ids>
get_sr_to_participating_irs(
    soro::vector_map<ir_id, interlocking_route> const& interlocking_routes,
    infrastructure const& infra) {
  utl::scoped_timer const participating_timer(
      "Creating station route to participating interlocking routes mapping");

  soro::vector_map<station_route::id, interlocking_route::ids>
      sr_to_participating_irs(infra->station_routes_.size());

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

soro::vector_map<station::id, interlocking_route::ids>
get_station_to_interlocking_routes(
    soro::vector_map<ir_id, interlocking_route> const& interlocking_routes,
    infrastructure const& infra) {
  soro::vector_map<station::id, interlocking_route::ids> station_to_irs(
      infra->stations_.size());

  for (auto const& ir : interlocking_routes) {
    for (auto const& sr_id : ir.station_routes_) {
      auto const sr = infra->station_routes_[sr_id];
      station_to_irs[sr->station_->id_].emplace_back(ir.id_);
    }
  }

  for (auto& station_irs : station_to_irs) {
    utl::erase_duplicates(station_irs);
  }

  utls::ensures([&] {
    for (auto const& station_irs : station_to_irs) {
      utls::sassert(utls::is_sorted(station_irs));
    }
  });

  return station_to_irs;
}

/*
 * Given a station route (not a signal one!) returns all internal signal
 * station routes. That is, all signal station routes that are completely
 * contained inside the station route.
 *
 * Number of internal signal station routes = Number of main signals in
 * station route - 1
 */
soro::vector_map<ir_id, interlocking_route> get_internal_interlocking_route(
    station_route::ptr sr) {
  soro::vector_map<ir_id, interlocking_route> interlocking_routes;

  for (auto [from, to] : utl::pairwise(sr->path_->main_signals_)) {
    interlocking_routes.emplace_back(interlocking_route{
        .id_ = interlocking_route::invalid(),
        .start_offset_ = from,
        .end_offset_ = utls::narrow<station_route::idx>(to + 1),
        .station_routes_ = {sr->id_}});
  }

  return interlocking_routes;
}

interlocking_route get_leading_interlocking_route(station_route::ptr const sr) {
  return {.id_ = interlocking_route::invalid(),
          .start_offset_ = 0,
          .end_offset_ = utls::narrow<station_route::idx>(
              sr->path_->main_signals_.front() + 1),
          .station_routes_ = {sr->id_}};
}

interlocking_route get_trailing_interlocking_route(
    station_route::ptr const sr) {
  return {.id_ = interlocking_route::invalid(),
          .start_offset_ = sr->path_->main_signals_.back(),
          .end_offset_ = sr->size(),
          .station_routes_ = {sr->id_}};
}

void gather_interlocking_routes_impl(
    interlocking_route ir, station_route::ptr const sr,
    infrastructure const& infra,
    soro::vector_map<ir_id, interlocking_route>& result) {

  if (sr->requires_etcs(infra->lines_) || sr->requires_lzb(infra->lines_)) {
    return;
  }

  ir.station_routes_.emplace_back(sr->id_);

  if (sr->can_end_an_interlocking(infra->station_route_graph_)) {
    ir.end_offset_ = sr->path_->main_signals_.empty()
                         ? sr->size()
                         : sr->path_->main_signals_.front() + 1;
    result.emplace_back(std::move(ir));
    return;
  }

  for (auto const& succ : infra->station_route_graph_.successors_[sr->id_]) {
    gather_interlocking_routes_impl(ir, succ, infra, result);
  }
}

void gather_interlocking_routes(
    interlocking_route const& ir, station_route::ptr const sr,
    infrastructure const& infra,
    soro::vector_map<ir_id, interlocking_route>& result) {
  for (auto const& succ : infra->station_route_graph_.successors_[sr->id_]) {
    gather_interlocking_routes_impl(ir, succ, infra, result);
  }
}

auto get_interlocking_routes_from_halt(station_route::ptr sr,
                                       infrastructure const& infra) {
  soro::vector_map<ir_id, interlocking_route> routes;

  // reserve 2^16 elements as around there is the maximum of interlocking
  // routes a single station route produces
  routes.reserve(std::pow(2, 16));

  interlocking_route initial_ir{.id_ = interlocking_route::invalid(),
                                .start_offset_ = station_route::invalid_idx(),
                                .end_offset_ = station_route::invalid_idx(),
                                .station_routes_ = {sr->id_}};

  auto const handle_stop_mode = [&](rs::stop_mode const stop_mode) {
    auto const halt_idx = sr->get_halt_idx(stop_mode);

    if (!halt_idx) return;

    auto const ms_it = utls::upper_bound(sr->path_->main_signals_, *halt_idx);

    initial_ir.start_offset_ = *halt_idx;

    if (ms_it != std::end(sr->path_->main_signals_)) {
      initial_ir.end_offset_ = *ms_it + 1;
      routes.emplace_back(initial_ir);
    } else {
      initial_ir.end_offset_ = sr->size();
      gather_interlocking_routes(initial_ir, sr, infra, routes);
    }
  };

  handle_stop_mode(rs::stop_mode::freight);
  handle_stop_mode(rs::stop_mode::passenger);

  utls::ensure(utls::all_of(routes, [](auto&& r) {
    return (r.start_offset_ != r.end_offset_ || r.station_routes_.size() > 1) &&
           r.start_offset_ != station_route::invalid_idx() &&
           r.end_offset_ != station_route::invalid_idx();
  }));

  return routes;
}

auto get_interlocking_routes_from_sr(station_route::ptr sr,
                                     infrastructure const& infra) {
  utls::expect(sr->can_start_an_interlocking(infra->station_route_graph_),
               "SR {} cannot start an interlocking route.", sr->id_);

  soro::vector_map<ir_id, interlocking_route> routes;

  // reserve 2^16 elements as around there is the maximum of interlocking
  // routes a single station route produces
  routes.reserve(std::pow(2, 16));

  //  auto const fill_paths = [&srg, &routes, &lines](
  //                              station_route::ptr const& route,
  //                              interlocking_route ir,
  //                              auto const& get_paths_ref) {
  //    if (route->requires_etcs(lines) || route->requires_lzb(lines)) {
  //      return;
  //    }
  //
  //    ir.station_routes_.emplace_back(route->id_);
  //
  //    if (route->can_end_an_interlocking(srg)) {
  //      ir.end_offset_ = route->path_->main_signals_.empty()
  //                           ? route->size()
  //                           : route->path_->main_signals_.front() + 1;
  //      routes.emplace_back(std::move(ir));
  //      return;
  //    }
  //
  //    for (auto const& neighbour : srg.successors_[route->id_]) {
  //      get_paths_ref(neighbour, ir, get_paths_ref);
  //    }
  //  };

  interlocking_route const initial_ir{
      .id_ = interlocking_route::invalid(),
      .start_offset_ = sr->path_->main_signals_.empty()
                           ? station_route::idx{0}
                           : sr->path_->main_signals_.back(),
      .end_offset_ = utls::narrow<station_route::idx>(sr->size()),
      .station_routes_ = {sr->id_}};

  gather_interlocking_routes(initial_ir, sr, infra, routes);
  //  for (auto const& succ : srg.successors_[sr->id_]) {
  //    fill_paths(succ, initial_ir, fill_paths);
  //  }

  return routes;
}

soro::vector_map<ir_id, interlocking_route> get_interlocking_routes(
    infrastructure const& infra) {
  utl::scoped_timer const routes_timer("generating interlocking routes");

  soro::vector_map<ir_id, interlocking_route> interlocking_routes;

  interlocking_routes.reserve(infra->station_routes_.size() * 30);

  for (auto const& sr : infra->station_routes_) {
    // add all interlocking routes that start with this station route.
    // do not generate them when etcs/lzb is required to use the station
    // route.
    if (sr->can_start_an_interlocking(infra->station_route_graph_) &&
        !sr->requires_etcs(infra->lines_) && !sr->requires_lzb(infra->lines_)) {
      utl::concat(interlocking_routes,
                  get_interlocking_routes_from_sr(sr, infra));
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

    utl::concat(interlocking_routes,
                get_interlocking_routes_from_halt(sr, infra));
  }

  interlocking_route::id current_id{0};
  for (auto& ir : interlocking_routes) {
    ir.id_ = current_id++;
  }

  return interlocking_routes;
}

void print_interlocking_stats(
    soro::vector_map<ir_id, interlocking_route> const& irs) {
  std::size_t length_in_srs = 0;

  for (auto const& ir : irs) {
    length_in_srs += ir.station_routes_.size();
  }

  uLOG(utl::info) << "Generated " << irs.size() << " interlocking routes.";
  uLOG(utl::info) << "Average length in station routes of an interlocking: "
                  << length_in_srs / irs.size();
}

critical_points get_critical_points(
    soro::vector_map<ir_id, interlocking_route> const& routes,
    infrastructure const& infra) {
  utl::scoped_timer const timer("creating critical points");

  critical_points result;

  for (auto const& ir : routes) {
    soro::vector<critical_point> cps;

    bool found_seotd = false;
    for (auto const& rn : ir.iterate(infra)) {
      if (rn.omitted_) {
        continue;
      }

      auto const type = rn.node_->type();

      if (type == type::SIMPLE_SWITCH || type == type::CROSS) {
        cps.emplace_back(rn.node_->element_->get_id(), type);
      }

      if (type == type::EOTD) {
        auto const& eotd =
            infra->graph_.get_element_data<struct eotd>(rn.node_);

        if (eotd.signal_) {
          found_seotd = true;
        } else if (found_seotd) {
          cps.emplace_back(rn.node_->element_->get_id(), type::EOTD);
        }
      }
    }

    result.emplace_back(std::move(cps));
  }

  return result;
}

interlocking get_interlocking(infrastructure_t const& infra_t) {
  infrastructure const infra(&infra_t);

  utl::scoped_timer const irs_timer("creating interlocking");

  interlocking irs;

  irs.routes_ = get_interlocking_routes(infra);

  irs.halting_at_ = get_halting_at(irs.routes_, infra);

  irs.starting_at_ = get_starting_at(irs.routes_, infra);
  irs.ending_at_ = get_ending_at(irs.routes_, infra);

  irs.sr_to_irs_ = get_sr_to_participating_irs(irs.routes_, infra);
  irs.station_to_irs_ = get_station_to_interlocking_routes(irs.routes_, infra);

  irs.critical_points_ = get_critical_points(irs.routes_, infra);

  print_interlocking_stats(irs.routes_);

  return irs;
}

}  // namespace soro::infra
