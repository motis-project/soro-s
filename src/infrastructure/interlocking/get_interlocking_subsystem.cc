#include "soro/infrastructure/interlocking/get_interlocking_subsystem.h"

#include "utl/enumerate.h"
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

auto get_halting_at(soro::vector<ir_ptr> const& ssrs) {
  soro::map<node::id, soro::vector<ir_ptr>> halting_at;

  auto add_halts = [&](auto const& ssr, auto const& halts) {
    for (auto const& halt_idx : halts) {
      auto const halt_id = ssr->nodes(halt_idx)->id_;
      insert_or(halting_at, halt_id, {ssr},
                [](auto&& vec, auto&& ssr_vec) { utls::append(vec, ssr_vec); });
    }
  };

  for (auto const& ssr : ssrs) {
    add_halts(ssr, ssr->passenger_halts_);
    add_halts(ssr, ssr->freight_halts_);
  }

  return halting_at;
}

auto get_starting_at(soro::vector<ir_ptr> const& ssrs) {
  soro::map<node::id, soro::vector<ir_ptr>> starting_at;

  for (auto const& ssr : ssrs) {
    auto const start_id = ssr->nodes().front()->id_;
    insert_or(starting_at, start_id, {ssr},
              [](auto&& vec, auto&& ssr_vec) { utls::append(vec, ssr_vec); });
  }

  return starting_at;
}

auto get_sr_to_participating_ssrs(soro::vector<ir_ptr> const& ssrs,
                                  base_infrastructure const& iss) {
  soro::vector<soro::vector<ir_ptr>> sr_to_participating_ssrs(
      iss.station_routes_.size());

  for (auto const& ssr : ssrs) {
    for (auto const& p_sr : ssr->station_routes_) {
      sr_to_participating_ssrs[p_sr->id_].emplace_back(ssr);
    }
  }

  return sr_to_participating_ssrs;
}

void add_node_to_ssr(interlocking_route& ssr, station_route::ptr const sr_ptr,
                     route_node const& rn) {
  auto const& node = sr_ptr->nodes(rn.node_idx_);

  if (rn.omitted_) {
    ssr.r_.omitted_nodes_.push_back(ssr.size());
  }

  if (node->is(type::HALT) &&
      sr_ptr->get_halt_node(rs::FreightTrain::YES) == node) {
    ssr.freight_halts_.push_back(ssr.size());
  }
  if (node->is(type::HALT) &&
      sr_ptr->get_halt_node(rs::FreightTrain::NO) == node) {
    ssr.passenger_halts_.push_back(ssr.size());
  }

  ssr.r_.nodes_.push_back(node);

  if (rn.extra_spl_.has_value()) {
    ssr.r_.extra_speed_limits_.push_back(*rn.extra_spl_.value());
  }

  if (ssr.station_routes_.empty() || ssr.station_routes_.back() != sr_ptr) {
    ssr.station_routes_.emplace_back(sr_ptr);
  }

  if (ssr.station_routes_.empty() || ssr.station_routes_.back() != sr_ptr) {
    ssr.station_routes_.push_back(sr_ptr);
  }
};

auto get_ssr_from_path(std::vector<station_route::ptr> const& path) {
  auto ssr = soro::make_unique<interlocking_route>();

  node_ptr last_node = nullptr;
  for (size_t path_idx = 0; path_idx < path.size(); ++path_idx) {
    auto const& path_sr = path[path_idx];

    // for the first sr iterate ms -> end, for the last iterate start -> ms
    // and for all inbetween iterate start -> end
    node::idx from = path_idx == 0 && !path_sr->main_signals_.empty()
                         ? path_sr->main_signals_.back()
                         : 0;

    node::idx const to =
        path_idx == path.size() - 1 && !path_sr->main_signals_.empty()
            ? path_sr->main_signals_.front() + 1
            : path_sr->size();

    // if path_sr is an out route the previous sr is an in route
    // if both halts (the end of the previous sr and the start of path_sr)
    // are on the same mileage we can skip these so they don't end up doubled
    // in the ssr path
    if (last_node != nullptr && path_sr->is_out_route()) {
      auto const pos = utls::find_position(path_sr->nodes(), last_node);
      from = pos != path_sr->nodes().size() ? static_cast<node::idx>(pos + 1)
                                            : from;
    }

    for (auto const& rn : path_sr->from_to(from, to, skip_omitted::OFF)) {
      add_node_to_ssr(*ssr, path_sr, rn);
    }

    last_node = ssr->nodes().back();
  }

  return ssr;
}

auto get_trailing_intermediate_ssr(station_route::ptr sr) {
  auto ssr = soro::make_unique<interlocking_route>();

  auto const from = sr->main_signals_.back();
  auto const to = sr->size();

  for (auto const& rn : sr->from_to(from, to, skip_omitted::OFF)) {
    add_node_to_ssr(*ssr, sr, rn);
  }

  return ssr;
}

auto get_leading_intermediate_ssr(station_route::ptr sr) {
  auto ssr = soro::make_unique<interlocking_route>();

  node::idx const from = 0;
  node::idx const to = sr->main_signals_.front() + 1;

  for (auto const& rn : sr->from_to(from, to, skip_omitted::OFF)) {
    add_node_to_ssr(*ssr, sr, rn);
  }

  return ssr;
}

/*
 * Given a station route (not a signal one!) returns all internal signal
 * station routes. That is, all signal station routes that are completely
 * contained inside the station route.
 *
 * Number of internal signal station routes = Number of main signals in
 * station route - 1
 */
auto get_internal_ssrs(station_route::ptr sr) {
  soro::vector<soro::unique_ptr<interlocking_route>> i_ssrs;

  for (auto [from, to] : utl::pairwise(sr->main_signals_)) {
    auto i_ssr = soro::make_unique<interlocking_route>();

    for (auto const& rn : sr->from_to(from, to + 1, skip_omitted::OFF)) {
      add_node_to_ssr(*i_ssr, sr, rn);
    }

    i_ssrs.emplace_back(std::move(i_ssr));
  }

  return i_ssrs;
}

/*
 * Given a station route sr (not a signal one!) and a station route graph
 * returns all possible signal station route paths starting at s_r. A path
 * has the form of a vector of station_route::ptr.
 *
 * Return is empty if sr does not contain a main signal.
 */
auto get_sr_paths(station_route::ptr sr, station_route_graph const& srg) {
  std::vector<std::vector<station_route::ptr>> paths;

  if (sr->id_ % 10000 == 0) {
    std::cout << sr->id_ << '/' << srg.successors_.size() << std::endl;
  }

  auto const& can_start_a_ssr = [&](station_route::ptr sr_ptr) {
    return !sr_ptr->main_signals_.empty() ||
           srg.predeccesors_[sr_ptr->id_].empty();
  };

  auto const& can_end_a_ssr = [&](station_route::ptr sr_ptr) {
    return !sr_ptr->main_signals_.empty() ||
           srg.successors_[sr_ptr->id_].empty();
  };

  auto const& fill_paths =
      [&](station_route::ptr const& route,
          std::vector<station_route::ptr> const& current_path,
          auto const& get_paths_ref) {
        auto new_path = current_path;

        new_path.emplace_back(route);

        if (can_end_a_ssr(route) && new_path.size() > 1) {
          paths.push_back(new_path);
          return;
        }

        for (auto const& neighbour : srg.successors_[route->id_]) {
          get_paths_ref(neighbour, new_path, get_paths_ref);
        }
      };

  //  std::set<station_route::id> ids;
  //  auto const& generate_graphviz = [&](station_route::ptr const route,
  //                                      auto const& generate_ref) {
  //    ids.insert(route->id_);
  //
  //    if (can_end_a_ssr(route) && route->id_ != sr->id_) {
  //      return;
  //    }
  //
  //    for (auto const& neighbour : srg.successors_[route->id_]) {
  //      generate_ref(neighbour, generate_ref);
  //    }
  //  };

  //  generate_graphviz(sr, generate_graphviz);
  //
  //  std::string dot = "";
  //  for (auto const& id : ids) {
  //    for (auto const& neigh : srg.successors_[id]) {
  //      dot += std::to_string(id) + " -> " + std::to_string(neigh->id_) +
  //      ";\n";
  //    }
  //  }

  //  std::cout << dot << std::endl;

  if (can_start_a_ssr(sr)) {
    fill_paths(sr, {}, fill_paths);
  }

  std::size_t accumulated = 0;
  for (auto const& path : paths) {
    accumulated += path.size();
  }

  if (paths.size() > 10'000) {
    std::cout << "Contributing more than 10k: " << paths.size() << std::endl;
  }

  return std::pair(paths.size(), accumulated);

  //  return paths;
}

auto get_signal_station_routes(base_infrastructure const& iss) {
  soro::vector<soro::unique_ptr<interlocking_route>> ssrs;

  soro::vector<soro::unique_ptr<interlocking_route>> starting_on_halt;
  soro::vector<soro::unique_ptr<interlocking_route>> ending_on_halt;

  auto const& srg = iss.station_route_graph_;

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

  //  auto const& add_ssrs_from_path = [&](station_route::ptr sr) {
  //    for (auto const& path : get_sr_paths(sr, srg)) {
  //      auto ssr = get_ssr_from_path(path);
  //      sort_on_halt(std::move(ssr));
  //    }
  //  };

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
  //
  //  auto const& add_internal_intermediate_ssrs = [&](station_route::ptr sr) {
  //    utls::append_move(ssrs, get_internal_ssrs(sr));
  //  };

  std::size_t irs_from_path = 0;
  std::size_t avg_length = 0;
  for (auto const& sr : iss.station_routes_) {
    auto const [paths, acc_length] = get_sr_paths(sr, srg);
    irs_from_path += paths;
    avg_length += acc_length;

    //    auto const paths = get_sr_paths()
    //    add_ssrs_from_path(sr);

    //    if (srg.successors_[sr->id_].empty()) {
    //      add_trailing_intermediate_ssr(sr);
    //    }
    //
    //    if (srg.predeccesors_[sr->id_].empty()) {
    //      add_leading_intermediate_ssr(sr);
    //    }
    //
    //    if (sr->main_signals_.size() > 1) {
    //      add_internal_intermediate_ssrs(sr);
    //    }
  }

  avg_length /= irs_from_path;
  std::cout << "IRs from paths: " << irs_from_path << std::endl;
  std::cout << "AVG length: " << avg_length << std::endl;

  // TODO(julian) this should actually happen during ssr construction
  auto const fill_eotd_data = [&](soro::unique_ptr<interlocking_route>& ssr) {
    for (node::idx idx = 0; idx < static_cast<node::idx>(ssr->nodes().size());
         ++idx) {
      auto const& node = ssr->nodes(idx);
      if (!node->is(type::EOTD)) {
        continue;
      }

      auto const& eotd_data =
          iss.graph_.element_data_[node->element_->id()].as<eotd>();

      if (eotd_data.signal_ && ssr->signal_eotd_ == node::INVALID_IDX) {
        ssr->signal_eotd_ = idx;
      }

      if (!eotd_data.signal_ && ssr->signal_eotd_ != node::INVALID_IDX) {
        ssr->route_eotds_.push_back(idx);
      }
    }
  };

  for (interlocking_route::id id = 0; id < ssrs.size(); ++id) {
    ssrs[id]->id_ = id;
    fill_eotd_data(ssrs[id]);
  }

  // TODO(julian) Decide what to do with the signal station routes ending or
  // starting on a halt
  uLOG(utl::info) << "Found " << ending_on_halt.size()
                  << " signal station routes ending on a halt";
  uLOG(utl::info) << "Found " << starting_on_halt.size()
                  << " signal station routes starting on a halt";

  return ssrs;
}

auto get_station_to_ssrs(soro::vector<ir_ptr> const& ssrs,
                         base_infrastructure const& iss) {
  soro::vector<soro::vector<ir_ptr>> station_to_ssrs(iss.stations_.size());

  for (auto const& ssr : ssrs) {
    utl::all(ssr->station_routes_) |
        utl::transform([](auto&& sr) { return sr->station_; }) |  // NOLINT
        utl::unique() |  // NOLINT
        utl::for_each([&](auto&& station) {
          station_to_ssrs[station->id_].push_back(ssr);
        });
  }

  return station_to_ssrs;
}

interlocking_subsystem get_interlocking_subsystem(
    base_infrastructure const& base_infra, bool const determine_conflicts) {
  utl::scoped_timer const ssr_timer("Signal station routes timer");
  interlocking_subsystem irs;

  irs.interlocking_route_store_ = get_signal_station_routes(base_infra);
  irs.interlocking_routes_ =
      soro::to_vec(irs.interlocking_route_store_,
                   [](auto const& ssr) -> ir_ptr { return ssr.get(); });

  irs.halting_at_ = get_halting_at(irs.interlocking_routes_);
  irs.starting_at_ = get_starting_at(irs.interlocking_routes_);
  irs.sr_to_participating_irs_ =
      get_sr_to_participating_ssrs(irs.interlocking_routes_, base_infra);
  irs.station_to_irs_ =
      get_station_to_ssrs(irs.interlocking_routes_, base_infra);

  if (determine_conflicts) {
    irs.exclusions_ = get_ssr_conflicts(base_infra, irs);
  }

  uLOG(utl::info) << "Created " << irs.interlocking_routes_.size()
                  << " signal station routes";

  return irs;
}

}  // namespace soro::infra
