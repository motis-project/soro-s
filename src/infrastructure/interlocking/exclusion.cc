#include "soro/infrastructure/interlocking/exclusion.h"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/erase_duplicates.h"
#include "utl/erase_if.h"
#include "utl/logging.h"
#include "utl/pipes.h"
#include "utl/timer.h"
#include "utl/to_vec.h"

#include "cista/containers/bitvec.h"

#include "soro/utls/algo/overlap.h"
#include "soro/utls/coroutine/collect.h"
#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

exclusion_path get_exclusion_path(station_route::path::ptr const path,
                                  node::idx const from, node::idx const to,
                                  infrastructure const& infra) {
  exclusion_path ep;

  element::ptr last_section_element = nullptr;
  element::ptr first_section_element = nullptr;

  for (auto idx = from; idx < to; ++idx) {
    auto const element = path->nodes_[idx]->element_;

    if (element->is_track_element()) {
      continue;
    }

    if (element->is_section_element()) {
      last_section_element = element;
    }

    if (first_section_element == nullptr) {
      first_section_element = element;
    }

    if (element->is_switch()) {
      ep.elements_.emplace_back(element->id());
    }
  }

  auto const first_element = path->nodes_[from]->element_;
  auto const last_element = path->nodes_[to - 1]->element_;

  if (first_element->is_track_element() && last_element->is_track_element() &&
      last_section_element == nullptr) {

    auto const only_sec_id =
        infra->graph_.element_id_to_section_ids_[first_element->id()].front();
    auto const& only_sec = infra->graph_.sections_[only_sec_id];

    auto const first_element_it =
        utls::find(only_sec.rising_order_, first_element);
    auto const last_element_it =
        utls::find(only_sec.rising_order_, last_element);

    auto from_it = std::min(first_element_it, last_element_it);
    auto const to_it = std::max(first_element_it, last_element_it) + 1;

    for (; from_it != to_it; ++from_it) {
      ep.elements_.emplace_back((*from_it)->id());
    }
  }

  if (last_element->is_track_element() && last_section_element != nullptr) {
    auto const last_sec_id =
        infra->graph_.element_id_to_section_ids_[last_element->id()].front();
    auto const& last_sec = infra->graph_.sections_[last_sec_id];

    auto const last_sec_ele_it =
        utls::find(last_sec.rising_order_, last_section_element);
    auto const last_element_it =
        utls::find(last_sec.rising_order_, last_element);

    auto from_it = std::min(last_sec_ele_it, last_element_it);
    auto const to_it = std::max(last_sec_ele_it, last_element_it) + 1;

    for (; from_it != to_it; ++from_it) {
      ep.elements_.emplace_back((*from_it)->id());
    }
  }

  if (first_element->is_track_element() && first_section_element != nullptr) {
    auto const first_sec_id =
        infra->graph_.element_id_to_section_ids_[first_element->id()].front();
    auto const& first_sec = infra->graph_.sections_[first_sec_id];

    auto const first_sec_ele_it =
        utls::find(first_sec.rising_order_, first_section_element);
    auto const first_ele_it =
        utls::find(first_sec.rising_order_, first_element);

    auto from_it = std::min(first_sec_ele_it, first_ele_it);
    auto const to_it = std::max(first_sec_ele_it, first_ele_it) + 1;

    for (; from_it != to_it; ++from_it) {
      ep.elements_.emplace_back((*from_it)->id());
    }
  }

  utls::sort(ep.elements_);

  return ep;
}

std::vector<exclusion_path> get_exclusion_paths_no_main_signals(
    station_route::path::ptr const path, infrastructure const& infra) {
  return {get_exclusion_path(path, 0, path->size(), infra)};
}

exclusion_path get_first_exclusion_path(station_route::path::ptr const path,
                                        infrastructure const& infra) {
  return get_exclusion_path(path, 0, path->main_signals_.front() + 1, infra);
}

exclusion_path get_last_exclusion_path(station_route::path::ptr const path,
                                       infrastructure const& infra) {
  return get_exclusion_path(path, path->main_signals_.front(), path->size(),
                            infra);
}

std::vector<exclusion_path> get_exclusion_paths_one_main_signal(
    station_route::path::ptr const path, infrastructure const& infra) {
  return {get_first_exclusion_path(path, infra),
          get_last_exclusion_path(path, infra)};
}

std::vector<exclusion_path> get_exclusion_paths_more_main_signals(
    station_route::path::ptr const path, infrastructure const& infra) {
  std::vector<exclusion_path> result;

  result.emplace_back(get_first_exclusion_path(path, infra));

  for (auto const [from, to] : utl::pairwise(path->main_signals_)) {
    result.emplace_back(get_exclusion_path(path, from, to + 1, infra));
  }

  result.emplace_back(get_last_exclusion_path(path, infra));

  return result;
}

std::vector<exclusion_path> get_exclusion_paths(
    station_route::path::ptr const path, infrastructure const& infra) {

  if (path->main_signals_.empty()) {
    return get_exclusion_paths_no_main_signals(path, infra);
  }

  if (path->main_signals_.size() == 1) {
    return get_exclusion_paths_one_main_signal(path, infra);
  }

  if (path->main_signals_.size() > 1) {
    return get_exclusion_paths_more_main_signals(path, infra);
  }

  utls::sassert(false);
  std::abort();
}

auto print_unique_exclusion_paths(
    std::vector<std::vector<exclusion_path>> const& eps) {

  auto const comp = [](auto&& ep1, auto&& ep2) {
    return ep1.elements_ < ep2.elements_;
  };

  std::vector<std::set<exclusion_path, decltype(comp)>> unique_eps(
      eps.size(), std::set<exclusion_path, decltype(comp)>(comp));

  std::vector<std::vector<exclusion_path>> result(eps.size());

  std::size_t x = 0;
  std::size_t total = 0;
  for (auto const& station_eps : eps) {
    unique_eps[x].insert(std::begin(station_eps), std::end(station_eps));

    result[x].insert(std::end(result[x]), std::begin(unique_eps[x]),
                     std::end(unique_eps[x]));
    total += unique_eps[x].size();

    ++x;
  }

  uLOG(utl::info) << "Unique EPs: " << total;

  return result;
}

std::vector<std::vector<exclusion_path>> get_exclusion_paths(
    infrastructure const& infra) {
  std::vector<std::vector<exclusion_path>> result(infra->stations_.size());

  for (auto const& path : infra->station_route_paths_) {
    auto const station =
        infra->element_to_station_.at(path->nodes_.front()->element_->id());

    auto const exclusion_paths = get_exclusion_paths(path, infra);

    utl::concat(result[station->id_], exclusion_paths);
  }

  return result;
}

bool excludes(exclusion_path const& sr1, exclusion_path const& sr2) {
  return utls::overlap(sr1.elements_, sr2.elements_);
}

auto get_exclusion_path_exclusions(
    exclusion_path const& ep, std::vector<exclusion_path> const& candidates) {
  std::vector<exclusion_path::id> exclusions;

  for (auto const& candidate : candidates) {
    if (candidate.id_ <= ep.id_) {
      continue;
    }

    if (excludes(ep, candidate)) {
      exclusions.emplace_back(candidate.id_);
    }
  }

  exclusions.emplace_back(ep.id_);

  if (exclusions.size() == 2557) {
    std::cout << "";
  }

  return exclusions;
}

std::vector<std::vector<exclusion_path::id>> get_exclusion_path_exclusions(
    infrastructure const& infra) {
  utl::manual_timer const sr_timer("Determining Exlusion Path Exclusions");

  auto exclusion_paths = get_exclusion_paths(infra);

  sr_timer.stop_and_print();

  exclusion_paths = print_unique_exclusion_paths(exclusion_paths);

  exclusion_path::id id = 0;
  for (auto& station_paths : exclusion_paths) {
    for (auto& path : station_paths) {
      path.id_ = id++;
    }
  }

  sr_timer.stop_and_print();

  auto const total_exclusion_paths =
      utls::accumulate(exclusion_paths, std::size_t{0},
                       [](auto&& acc, auto&& station_exclusion_paths) {
                         return acc + station_exclusion_paths.size();
                       });

  std::vector<std::vector<exclusion_path::id>> exclusion_path_exclusions(
      total_exclusion_paths);

  uLOG(utl::info) << "Total exclusion paths: " << total_exclusion_paths;

  for (auto const& station_exclusion_paths : exclusion_paths) {
    for (auto const& exclusion_path : station_exclusion_paths) {
      exclusion_path_exclusions[exclusion_path.id_] =
          get_exclusion_path_exclusions(exclusion_path,
                                        station_exclusion_paths);
    }
  }

  sr_timer.stop_and_print();
  //
  //  for (auto const& station_route : infra.station_routes_) {
  //    auto const& exclusions = station_route_exclusions[station_route->id_];
  //
  //    for (auto const excluded_route : exclusions) {
  //      if (excluded_route == station_route->id_) {
  //        break;
  //      }
  //
  //      station_route_exclusions[excluded_route].emplace_back(station_route->id_);
  //    }
  //  }

  //  sr_timer.stop_and_print();

  for (auto& exclusions : exclusion_path_exclusions) {
    utls::sort(exclusions);
  }

  sr_timer.stop_and_print();

  return exclusion_path_exclusions;
}

std::vector<section::id> get_connecting_sections(element_ptr e1, element_ptr e2,
                                                 graph const& net) {
  auto const& e1_sections = net.element_id_to_section_ids_[e1->id()];
  auto const& e2_sections = net.element_id_to_section_ids_[e2->id()];

  return utl::all(e1_sections) | utl::remove_if([&](auto&& section_id) {
           return !utls::contains(e2_sections, section_id);
         }) |
         utl::vec();
}

namespace detail {

bool excludes(station_route::id const sr1, station_route::id const sr2,
              std::vector<std::vector<node::id>> const& sorted_nodes) {
  // TODO(julian) this could be improved by only checking for overlapping
  // sections, instead of overlapping nodes
  return utls::overlap(sorted_nodes[sr1], sorted_nodes[sr2]);
}

auto get_station_route_exclusions(
    station_route::ptr const station_route,
    std::vector<std::vector<node::id>> const& sorted_station_routes) {
  std::vector<station_route::id> exclusions;

  for (auto const& [_, candidate] : station_route->station_->station_routes_) {
    if (station_route->id_ <= candidate->id_) {
      continue;
    }

    if (excludes(station_route->id_, candidate->id_, sorted_station_routes)) {
      exclusions.emplace_back(candidate->id_);
    }
  }

  exclusions.emplace_back(station_route->id_);

  return exclusions;
}

std::vector<std::vector<node::id>> get_sorted_station_route_nodes(
    infrastructure_t const& infra) {
  utl::scoped_timer const sorted_timer("Creating sorted station route nodes");
  return utl::to_vec(infra.station_routes_, [](auto&& station_route) {
    std::vector<node::id> node_ids =
        utl::all(station_route->nodes()) |
        utl::transform([](auto&& n_ptr) { return n_ptr->id_; }) | utl::vec();
    utls::sort(node_ids);
    return node_ids;
  });
}

std::vector<std::vector<station_route::id>> get_station_route_exclusions(
    infrastructure_t const& infra) {
  utl::manual_timer const sr_timer("Determining Station Route Exclusions");

  std::vector<std::vector<node::id>> const sorted_station_routes =
      get_sorted_station_route_nodes(infra);

  auto station_route_exclusions =
      utl::to_vec(infra.station_routes_, [&](auto&& station_route) {
        return get_station_route_exclusions(station_route,
                                            sorted_station_routes);
      });

  sr_timer.stop_and_print();

  for (auto const& station_route : infra.station_routes_) {
    auto const& exclusions = station_route_exclusions[station_route->id_];

    for (auto const excluded_route : exclusions) {
      if (excluded_route == station_route->id_) {
        break;
      }

      station_route_exclusions[excluded_route].emplace_back(station_route->id_);
    }
  }

  sr_timer.stop_and_print();

  for (auto& exclusions : station_route_exclusions) {
    utls::sort(exclusions);
  }

  sr_timer.stop_and_print();

  return station_route_exclusions;
}

}  // namespace detail

soro::vector<soro::vector<interlocking_route::id>>
get_interlocking_route_exclusions(interlocking_subsystem const&,
                                  infrastructure_t const& infra) {
  utl::scoped_timer const timer("Calculating interlocking route exclusions");

  auto const station_route_exclusions =
      detail::get_station_route_exclusions(infra);

  return {};
}

}  // namespace soro::infra
