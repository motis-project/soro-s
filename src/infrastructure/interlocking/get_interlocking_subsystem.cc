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
//
// void add_node_to_ssr(interlocking_route& ssr, station_route::ptr const
// sr_ptr,
//                      route_node const& rn) {
//   auto const& node = sr_ptr->nodes(rn.node_idx_);
//
//   if (rn.omitted_) {
//     ssr.r_.omitted_nodes_.push_back(ssr.size());
//   }
//
//   if (node->is(type::HALT) &&
//       sr_ptr->get_halt_node(rs::FreightTrain::YES) == node) {
//     ssr.freight_halts_.push_back(ssr.size());
//   }
//   if (node->is(type::HALT) &&
//       sr_ptr->get_halt_node(rs::FreightTrain::NO) == node) {
//     ssr.passenger_halts_.push_back(ssr.size());
//   }
//
//   ssr.r_.nodes_.push_back(node);
//
//   if (rn.extra_spl_.has_value()) {
//     ssr.r_.extra_speed_limits_.push_back(*rn.extra_spl_.value());
//   }
//
//   if (ssr.station_routes_.empty() || ssr.station_routes_.back() != sr_ptr) {
//     ssr.station_routes_.emplace_back(sr_ptr);
//   }
//
//   if (ssr.station_routes_.empty() || ssr.station_routes_.back() != sr_ptr) {
//     ssr.station_routes_.push_back(sr_ptr);
//   }
// };
//
// auto get_ssr_from_path(std::vector<station_route::ptr> const& path) {
//   auto ssr = soro::make_unique<interlocking_route>();
//
//   node_ptr last_node = nullptr;
//   for (size_t path_idx = 0; path_idx < path.size(); ++path_idx) {
//     auto const& path_sr = path[path_idx];
//
//     // for the first sr iterate ms -> end, for the last iterate start -> ms
//     // and for all inbetween iterate start -> end
//     node::idx from = path_idx == 0 && !path_sr->main_signals_.empty()
//                          ? path_sr->main_signals_.back()
//                          : 0;
//
//     node::idx const to =
//         path_idx == path.size() - 1 && !path_sr->main_signals_.empty()
//             ? path_sr->main_signals_.front() + 1
//             : path_sr->size();
//
//     // if path_sr is an out route the previous sr is an in route
//     // if both halts (the end of the previous sr and the start of path_sr)
//     // are on the same mileage we can skip these so they don't end up doubled
//     // in the ssr path
//     if (last_node != nullptr && path_sr->is_out_route()) {
//       auto const pos = utls::find_position(path_sr->nodes(), last_node);
//       from = pos != path_sr->nodes().size() ? static_cast<node::idx>(pos + 1)
//                                             : from;
//     }
//
//     for (auto const& rn : path_sr->from_to(from, to, skip_omitted::OFF)) {
//       add_node_to_ssr(*ssr, path_sr, rn);
//     }
//
//     last_node = ssr->nodes().back();
//   }
//
//   return ssr;
// }
//
// auto get_trailing_intermediate_ssr(station_route::ptr sr) {
//   auto ssr = soro::make_unique<interlocking_route>();
//
//   auto const from = sr->main_signals_.back();
//   auto const to = sr->size();
//
//   for (auto const& rn : sr->from_to(from, to, skip_omitted::OFF)) {
//     add_node_to_ssr(*ssr, sr, rn);
//   }
//
//   return ssr;
// }

// auto get_leading_intermediate_ssr(station_route::ptr sr) {
//   auto ssr = soro::make_unique<interlocking_route>();
//
//   node::idx const from = 0;
//   node::idx const to = sr->main_signals_.front() + 1;
//
//   for (auto const& rn : sr->from_to(from, to, skip_omitted::OFF)) {
//     add_node_to_ssr(*ssr, sr, rn);
//   }
//
//   return ssr;
// }

// auto get_internal_ssrs(station_route::ptr sr) {
//   soro::vector<soro::unique_ptr<interlocking_route>> i_ssrs;
//
//   for (auto [from, to] : utl::pairwise(sr->main_signals_)) {
//     auto i_ssr = soro::make_unique<interlocking_route>();
//
//     for (auto const& rn : sr->from_to(from, to + 1, skip_omitted::OFF)) {
//       add_node_to_ssr(*i_ssr, sr, rn);
//     }
//
//     i_ssrs.emplace_back(std::move(i_ssr));
//   }
//
//   return i_ssrs;
// }

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
                           .station_routes_ = {sr->id_},
                           .length_ = si::INVALID<si::length>});
  }

  return interlocking_routes;
}

interlocking_route get_leading_interlocking_route(station_route::ptr const sr) {
  return {.id_ = interlocking_route::INVALID,
          .start_offset_ = 0,
          .end_offset_ =
              static_cast<node::idx>(sr->path_->main_signals_.front() + 1),
          .station_routes_ = {sr->id_},
          .length_ = si::INVALID<si::length>};
}

interlocking_route get_trailing_interlocking_route(
    station_route::ptr const sr) {
  return {.id_ = interlocking_route::INVALID,
          .start_offset_ = sr->path_->main_signals_.back(),
          .end_offset_ = sr->size(),
          .station_routes_ = {sr->id_},
          .length_ = si::INVALID<si::length>};
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
      .station_routes_ = {sr->id_},
      .length_ = si::INVALID<si::length>};

  for (auto const& succ : srg.successors_[sr->id_]) {
    fill_paths(succ, initial_ir, fill_paths);
  }

  return routes;
}

si::length get_length(interlocking_route const&, infrastructure_t const&) {
  return si::INVALID<si::length>;
  // TODO(julian) reenable this
  //  if (ir.station_routes_.size() == 1) {
  //    return get_path_length_from_elements(utls::coro_map(
  //        infra.station_routes_[ir.station_routes_.front()]->from_to(
  //            ir.start_offset_, ir.end_offset_),
  //        [](auto&& rn) { return rn.node_->element_; }));
  //  }
  //
  //  si::length length = si::ZERO<si::length>;
  //  length += get_path_length_from_elements(utls::coro_map(
  //      infra.station_routes_[ir.station_routes_.front()]->from(ir.start_offset_),
  //      [](auto&& rn) { return rn.node_->element_; }));
  //
  //  for (auto i = 1UL; i < ir.station_routes_.size() - 1; ++i) {
  //    length += infra.station_routes_[ir.station_routes_[i]]->length_;
  //  }
  //
  //  length += get_path_length_from_elements(utls::coro_map(
  //      infra.station_routes_[ir.station_routes_.back()]->to(ir.end_offset_),
  //      [](auto&& rn) { return rn.node_->element_; }));
  //
  //  return length;
}

soro::vector<interlocking_route> get_interlocking_routes(
    infrastructure_t const& infra) {
  utl::scoped_timer const routes_timer("Generating Interlocking Routes");

  soro::vector<interlocking_route> interlocking_routes;
  interlocking_routes.reserve(infra.station_routes_.size() * 30);

  /*
   * Sort the signal station route into the correct bucket.
   * Starts on halt, ends on halt or neither.
   * Starting and ending on halt are special cases that need extra attention,
   * as they are not conforming to the default definition of a station route:
   * Starting and ending on a main signal.
   */
  //  auto const sort_on_halt = [&](auto&& ssr) {
  //    using uptr = soro::unique_ptr<interlocking_route>;
  //    if (ssr->nodes().back()->is(type::HALT)) {
  //      ending_on_halt.emplace_back(std::forward<uptr>(ssr));
  //    } else if (ssr->nodes().front()->is(type::HALT)) {
  //      starting_on_halt.emplace_back(std::forward<uptr>(ssr));
  //    } else {
  //      ssrs.emplace_back(std::forward<uptr>(ssr));
  //    }
  //  };
  //
  //
  //  auto const& add_trailing_intermediate_ssr = [&](station_route::ptr sr) {
  //    if (sr->main_signals_.empty()) {
  //      return;
  //    }
  //
  //    auto trailing_ssr = get_trailing_intermediate_ssr(sr);
  //    sort_on_halt(std::move(trailing_ssr));
  //  };
  //
  //  auto const& add_leading_intermediate_ssr = [&](station_route::ptr sr) {
  //    if (sr->main_signals_.empty()) {
  //      return;
  //    }
  //
  //    auto leading_ssr = get_leading_intermediate_ssr(sr);
  //    sort_on_halt(std::move(leading_ssr));
  //  };

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

    //    if (srg.successors_[sr->id_].empty()) {
    //      add_trailing_intermediate_ssr(sr);
    //    }
    //
    //    if (srg.predeccesors_[sr->id_].empty()) {
    //      add_leading_intermediate_ssr(sr);
    //    }
  }

  interlocking_route::id current_id = 0;
  for (auto& ir : interlocking_routes) {
    ir.id_ = current_id++;
    ir.length_ = get_length(ir, infra);
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

  // TODO(julian) Decide what to do with the signal station routes ending or
  // starting on a halt
  // uLOG(utl::info) << "Found " << ending_on_halt.size()
  //                << " signal station routes ending on a halt";
  // uLOG(utl::info) << "Found " << starting_on_halt.size()
  //                << " signal station routes starting on a halt";
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
