#include "soro/infrastructure/interlocking/exclusion.h"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/erase_duplicates.h"
#include "utl/logging.h"
#include "utl/pipes.h"
#include "utl/timer.h"

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

    auto const first_element_it = utls::find(only_sec.elements_, first_element);
    auto const last_element_it = utls::find(only_sec.elements_, last_element);

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
        utls::find(last_sec.elements_, last_section_element);
    auto const last_element_it = utls::find(last_sec.elements_, last_element);

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
        utls::find(first_sec.elements_, first_section_element);
    auto const first_ele_it = utls::find(first_sec.elements_, first_element);

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
  return {get_exclusion_path(path, 0, path->nodes_.size(), infra)};
}

exclusion_path get_first_exclusion_path(station_route::path::ptr const path,
                                        infrastructure const& infra) {
  return get_exclusion_path(path, 0, path->main_signals_.front() + 1, infra);
}

exclusion_path get_last_exclusion_path(station_route::path::ptr const path,
                                       infrastructure const& infra) {
  return get_exclusion_path(path, path->main_signals_.front(),
                            path->nodes_.size(), infra);
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

// soro::vector<section::id> get_sections(station_route::path const& sr_path,
//                                        base_infrastructure const& iss) {
//   auto section_elements =
//       utl::all(sr_path.nodes_) |
//       utl::transform([](auto&& node) { return node->element_; }) |
//       utl::remove_if([](auto&& e) { return e->is_track_element(); }) |
//       utl::vec();
//
//   soro::vector<section::id> section_ids;
//   if (ssr.starts_on_ms()) {
//     section_ids.push_back(
//         iss.graph_
//             .element_id_to_section_ids_[ssr.nodes().front()->element_->id()]
//             .front());
//   }
//
//   for (auto const [e1, e2] : utl::pairwise(section_elements)) {
//     auto const connecting_sections =
//         get_connecting_sections(e1, e2, iss.graph_);
//
//     utl::verify(!connecting_sections.empty(),
//                 "All section elements in a signal station route must be "
//                 "connected pairwise");
//
//     if (connecting_sections.size() == 1) {
//       section_ids.push_back(connecting_sections.front());
//       continue;
//     }
//
//     // handle the following case:
//     // two elements are connected more than once, so there exist more than
//     one
//     // connecting section. Appraoch: Just check which section contains a
//     track
//     // element from nodes_. This requires that there is at least one track
//     // element in the section
//     auto it = utls::find_if(connecting_sections, [&](auto&& section_id) {
//       auto const& section = iss.graph_.sections_[section_id];
//
//       utl::verify(section.elements_.size() > 2,
//                   "Need at least a single track element in this section, so "
//                   "the following approach works.");
//
//       auto const eles = ssr.elements();
//       return utls::overlap_non_sorted(std::cbegin(section.elements_) + 1,
//                                       std::cend(section.elements_) - 1,
//                                       std::cbegin(eles), std::cend(eles));
//     });
//
//     if (it != std::cend(connecting_sections)) {
//       section_ids.push_back(*it);
//       continue;
//     }
//
//     // Could not find a suitable section from the set of connecting sections.
//     // Conclusion: The section we use contains no element we care about
//     // this means it only contains track elements facing the opposite
//     direction it = utls::find_if(connecting_sections, [&, e = e1](auto&&
//     section_id) {
//       auto const& section = iss.graph_.sections_[section_id];
//       bool const rising = section.elements_.front()->id() == e->id();
//
//       return std::all_of(std::cbegin(section.elements_) + 1,
//                          std::cend(section.elements_) - 1,
//                          [&](auto&& s_e) { return s_e->rising() != rising;
//                          });
//     });
//
//     utl::verify(it != std::cend(connecting_sections),
//                 "Could not find a suitable connection in connecting
//                 sections");
//
//     section_ids.push_back(*it);
//   }
//
//   if (ssr.ends_on_ms() || ssr.nodes().back()->is(type::HALT)) {
//     section_ids.push_back(
//         iss.graph_
//             .element_id_to_section_ids_[ssr.nodes().back()->element_->id()]
//             .front());
//   }
//
//   if (section_ids.size() == 2 && section_ids.front() == section_ids.back()) {
//     section_ids.erase(std::end(section_ids) - 1);
//   }
//
//   return section_ids;
// }
//
//  soro::vector<element_ptr> get_exclusion_elements(
//      interlocking_route const& ssr, base_infrastructure const& iss) {
//    soro::vector<element_ptr> elements;
//
//    auto section_ids = get_sections(ssr, iss);
//
//    // gather every section element used by the signal station route
//    auto section_elements =
//        utl::all(ssr.nodes()) |
//        utl::transform([](auto&& node) { return node->element_; }) |
//        utl::remove_if(
//            [](auto&& element) { return element->is_track_element(); }) |
//        utl::unique() | utl::vec();
//
//    auto const handle_first_section = [&]() {
//      auto const& first_sec = iss.graph_.sections_[section_ids.front()];
//      auto const& first_sec_element = section_elements.front();
//
//      utls::append(elements, first_sec.elements_);
//
//      bool const reverse =
//          ssr.starts_on_ms()
//              ? first_sec.elements_.front()->id() == first_sec_element->id()
//              : first_sec.elements_.front()->id() != first_sec_element->id();
//
//      if (reverse) {
//        std::reverse(std::begin(elements), std::end(elements));
//      }
//    };
//
//    // handle [2, ..., n - 1] sections
//    auto const handle_sections = [&]() {
//      auto from_it = std::cbegin(section_ids) + 1;
//      auto to_it = std::cend(section_ids) - 1;
//
//      auto curr_sec_element_idx = ssr.starts_on_ms() ? 1UL : 2UL;
//      for (auto const& section_id : utls::make_range(from_it, to_it)) {
//        auto const& section = iss.graph_.sections_[section_id];
//        auto const& section_element = section_elements[curr_sec_element_idx];
//
//        utls::append(elements, section.elements_);
//
//        bool const reverse =
//            section.elements_.front()->id() == section_element->id();
//
//        if (reverse) {
//          std::reverse(std::end(elements) -
//                           static_cast<ptrdiff_t>(section.elements_.size()),
//                       std::end(elements));
//        }
//
//        ++curr_sec_element_idx;
//      }
//    };
//
//    auto const handle_last_section = [&]() {
//      auto const& last_sec = iss.graph_.sections_[section_ids.back()];
//      auto const last_sec_element = section_elements.back();
//
//      utls::append(elements, last_sec.elements_);
//
//      auto const reverse =
//          ssr.ends_on_ms()
//              ? last_sec.elements_.front()->id() != last_sec_element->id()
//              : last_sec.elements_.front()->id() == last_sec_element->id();
//
//      if (reverse) {
//        std::reverse(std::end(elements) -
//                         static_cast<ptrdiff_t>(last_sec.elements_.size()),
//                     std::end(elements));
//      }
//    };
//
//    auto const handle_single_section_ssr = [&]() {
//      auto const& section = iss.graph_.sections_[section_ids.front()];
//
//      utls::append(elements, section.elements_);
//
//      auto start = utls::find(section.elements_,
//      ssr.nodes().front()->element_); auto end = utls::find(section.elements_,
//      ssr.nodes().back()->element_);
//
//      if (std::distance(end, start) > 0) {
//        std::reverse(std::begin(elements), std::end(elements));
//      }
//    };
//
//    if (section_ids.size() == 1) {
//      handle_single_section_ssr();
//    }
//
//    if (section_ids.size() > 1) {
//      handle_first_section();
//    }
//
//    if (section_ids.size() > 2) {
//      handle_sections();
//    }
//
//    if (section_ids.size() > 1) {
//      handle_last_section();
//    }
//
//    // remove the exclusion elements until the original first element
//    utls::remove_until(elements, [&](auto&& element) {
//      return element->id() == ssr.nodes().front()->element_->id();
//    });
//
//    // remove the exclusion elements after the original last element
//    utls::remove_after(elements, [&](auto&& element) {
//      return element->id() == ssr.nodes().back()->element_->id();
//    });
//
//    utls::unique_erase(elements);
//    return elements;
//  }
//
///*
// * Returns for a given signal station route all potential conflicting signal
// * station routes.
// *
// * For now it does the following: Gather every station that is touched by
// * the given signal station route. Then, gather every signal station route
// * touching the gathered stations. Afterwards erase all duplicates.
// *
// */
// auto get_conflict_candidates(ir_ptr ssr, interlocking_subsystem const& ssr_m)
// {
//  soro::vector<ir_ptr> candidates;
//
//  utl::all(ssr->station_routes_) |
//      utl::transform([](auto&& sr) { return sr->station_; }) |  // NOLINT
//      utl::unique() |  // NOLINT
//      utl::transform(
//          [&](auto&& station) { return ssr_m.station_to_irs_[station->id_]; })
//          |
//      utl::for_each([&](auto&& ssr_vec) { utls::append(candidates, ssr_vec);
//      });
//
//  utls::sort(candidates);
//  utls::unique_erase(candidates);
//
//  return candidates;
//}
//
// struct exclusion_matrix {
//  void set(std::size_t const idx1, std::size_t const idx2, bool const val) {
//    auto const min = std::min(idx1, idx2);
//    auto const max = std::max(idx1, idx2);
//    bits_[min][max - min] = val;
//  }
//
//  bool test(std::size_t const idx1, std::size_t const idx2) const {
//    auto const min = std::min(idx1, idx2);
//    auto const max = std::max(idx1, idx2);
//    return bits_[min][max - min];
//  }
//
//  soro::vector<std::vector<bool>> bits_;
//};
//
// exclusion_matrix make_exclusion_matrix(std::size_t const entries) {
//  exclusion_matrix em;
//
//  em.bits_.resize(entries);
//  for (auto i = 0UL; i < entries; ++i) {
//    em.bits_[i].resize(entries - i, false);
//    //    em.bits_[i] = std::vector<bool>(entries - i, false);
//  }
//
//  return em;
//}
//
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

auto get_sorted_station_route_nodes(infrastructure_t const& infra) {
  utl::scoped_timer const sorted_timer("Creating sorted station route nodes");
  return soro::to_vec(infra.station_routes_, [](auto&& station_route) {
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
      soro::to_vec(infra.station_routes_, [&](auto&& station_route) {
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

auto generate_ir_exclusions_from_sr_exclusions(
    interlocking_route const& ir, interlocking_subsystem const& irs,
    std::vector<std::vector<station_route::id>> const& sr_exclusions,
    infrastructure_t const& infra) {

  if (ir.id_ % 10'000 == 0) {
    std::cout << "Genearting route exclusions: " << ir.id_ << std::endl;
  }

  std::vector<interlocking_route::id> excluded_irs;

  if (ir.id_ > 30'000) {
    return excluded_irs;
  }

  for (auto i = 1U; i < ir.station_routes_.size() - 1; ++i) {
    auto const sr_id = ir.station_routes_[i];
    for (auto const& excluded_station_route : sr_exclusions[sr_id]) {
      utl::concat(excluded_irs,
                  irs.sr_to_participating_irs_[excluded_station_route]);
    }
  }

  auto x = [&infra, &irs, &sr_exclusions, &excluded_irs, &ir](
               station_route::id const sr_id,
               std::vector<node::id> const& ir_nodes) {
    for (auto const& excluded_id : sr_exclusions[sr_id]) {
      for (auto const candidate_id :
           irs.sr_to_participating_irs_[excluded_id]) {

        if (ir.id_ <= candidate_id) {
          continue;
        }

        auto const candidate = irs.interlocking_routes_[candidate_id];

        // two ssrs which follow each other are not in conflict, although they
        // share a single element: the last/first one (the main signal).
        //        if (ir.follows(candidate) || candidate.follows(ir)) {
        //          continue;
        //        }

        auto const conflict_in_first = excluded_id == candidate.first_sr_id();
        auto const conflict_in_last = excluded_id == candidate.last_sr_id();

        if (!(conflict_in_first || conflict_in_last)) {
          continue;
        }

        std::vector<node::id> candidate_nodes;
        if (conflict_in_first) {
          candidate_nodes = utls::collect<std::vector<node::id>>(
              candidate.first_sr_nodes(infra));
        } else {
          candidate_nodes = utls::collect<std::vector<node::id>>(
              candidate.last_sr_nodes(infra));
        }
        utls::sort(candidate_nodes);

        if (utls::overlap(ir_nodes, candidate_nodes)) {
          excluded_irs.emplace_back(candidate_id);
        }
      }
    }
  };

  auto first_sr_nodes =
      utls::collect<std::vector<node::id>>(ir.first_sr_nodes(infra));
  utls::sort(first_sr_nodes);

  auto last_sr_nodes =
      utls::collect<std::vector<node::id>>(ir.last_sr_nodes(infra));
  utls::sort(last_sr_nodes);

  x(ir.first_sr_id(), first_sr_nodes);
  x(ir.last_sr_id(), last_sr_nodes);

  excluded_irs.emplace_back(ir.id_);
  utl::erase_duplicates(excluded_irs);

  return excluded_irs;
}

// bool exclusion(interlocking_route const& ir1, interlocking_route const& ir2)
// {
//   // a ssr is in conflict with itself
//   if (ir1 == ir2) {
//     return true;
//   }
//
//   // two ssrs which follow each other are not in conflict, although they
//   // share a single element: the last/first one (the main signal).
//   if (ir1.follows(ir2) || ir2.follows(ir1)) {
//     return false;
//   }
//
//   return utls::overlap(sorted_exclusion_elements[ssr1->id_],
//                        sorted_exclusion_elements[ssr2->id_]);
// }
//
// auto determine_exclusions(
//     interlocking_route const& ir,
//     std::vector<std::vector<station_route::id>> const&
//     station_route_exclusions, base_infrastructure const& infra) {
//
//   std::vector<interlocking_route::id> candidates;
//   for (auto const& sr_id : ir.station_routes_) {
//     for (auto const& excluded_sr : station_route_exclusions[sr_id]) {
//       utl::concat(candidates,
//                   infra.interlocking_.sr_to_participating_irs_[excluded_sr]);
//     }
//   }
//
//   utl::erase_duplicates(candidates);
//
//   utl::erase_if(candidates, [&](auto&& candidate_id) {
//     return !exclusion(ir,
//                       infra.interlocking_.interlocking_routes_[candidate_id]);
//   });
//   //  for (auto const candidate : candidates) {
//   //
//   //  }
// }

soro::vector<soro::vector<interlocking_route::id>>
get_interlocking_route_exclusions(
    interlocking_subsystem const& irs,
    std::vector<std::vector<station_route::id>> const& sr_exclusions,
    infrastructure_t const& infra) {
  utl::scoped_timer const timer("Determining Interlocking Route Exclusions");
  return soro::to_vec(irs.interlocking_routes_, [&](auto&& ir) {
    return generate_ir_exclusions_from_sr_exclusions(ir, irs, sr_exclusions,
                                                     infra);
  });
}

template <typename VecVec>
void print_vecvec_stats(VecVec const& vecvec) {
  std::size_t avg = 0;
  std::size_t max = 0;
  std::size_t min = 0;
  for (auto const& vec : vecvec) {
    avg += vec.size();
    min = std::min(min, vec.size());
    max = std::max(max, vec.size());
  }

  std::cout << "Min: " << min << " Max: " << max
            << " Avg: " << avg / vecvec.size() << std::endl;
}

soro::vector<soro::vector<interlocking_route::id>>
get_interlocking_route_exclusions(interlocking_subsystem const& irs,
                                  infrastructure_t const& infra) {
  utl::scoped_timer const timer("Calculating interlocking route exclusions");

  std::size_t max_station_route_station = 0;
  for (auto const station : infra.stations_) {
    max_station_route_station =
        std::max(max_station_route_station, station->station_routes_.size());
  }
  std::cout << "max station route station size: " << max_station_route_station
            << '\n';

  auto const station_route_exclusions =
      detail::get_station_route_exclusions(infra);

  //  std::size_t total_exclusions = 0;
  //  for (auto const& exclusions : station_route_exclusions) {
  //    total_exclusions += exclusions.size();
  //  }
  //
  //  std::cout << "Avg exclusions for a single station route: "
  //            << total_exclusions / station_route_exclusions.size() << '\n';
  std::cout << "Station Route Exclusions Stats:\n";
  print_vecvec_stats(station_route_exclusions);
  std::cout << "SR to participating IR Stats:\n";
  print_vecvec_stats(irs.sr_to_participating_irs_);

  for (auto const& [id, part_irs] :
       utl::enumerate(irs.sr_to_participating_irs_)) {
    if (part_irs.size() > 1'000'000) {
      std::cout << id << '\n';
    }
  }

  std::size_t total_participating_irs = 0;
  for (auto const& participating_irs : irs.sr_to_participating_irs_) {
    total_participating_irs += participating_irs.size();
  }

  std::cout << "Avg participating irs: "
            << total_participating_irs / irs.interlocking_routes_.size()
            << std::endl;

  std::size_t total_sr_exclusions_per_ir = 0;
  std::size_t total_ir_exclusion_candidates_per_ir = 0;
  std::size_t total_ir_exclusion_candidates_per_ir_no_doubles = 0;
  std::size_t total_certain_ir_exclusions_per_ir = 0;
  std::size_t total_certain_ir_exclusions_per_ir_no_doubles = 0;
  for (auto const& ir : irs.interlocking_routes_) {
    //    std::vector<interlocking_route::id> candidates;
    std::size_t certian_conflicts = 0;

    for (auto const& sr_id : ir.station_routes_) {
      total_sr_exclusions_per_ir += station_route_exclusions[sr_id].size();
      total_ir_exclusion_candidates_per_ir +=
          irs.sr_to_participating_irs_[sr_id].size();

      //      utl::concat(candidates, irs.sr_to_participating_irs_[sr_id]);
    }

    //    utl::erase_duplicates(candidates);
    //    total_ir_exclusion_candidates_per_ir_no_doubles += candidates.size();

    //    candidates.clear();

    if (ir.station_routes_.size() > 2) {
      for (auto i = 1U; i < ir.station_routes_.size() - 1; ++i) {
        auto const sr_id = ir.sr_id(i);
        total_certain_ir_exclusions_per_ir +=
            irs.sr_to_participating_irs_[sr_id].size();
        certian_conflicts += irs.sr_to_participating_irs_[sr_id].size();
        //        if (certian_conflicts > 200'000) {
        //          std::cout << "hi\n";
        //        }
        //        utl::concat(candidates, irs.sr_to_participating_irs_[sr_id]);
      }
    }

    //    std::cout << "Certain Conflicts: " << certian_conflicts << " from "
    //              << ir.station_routes_.size() << " station routes " <<
    //              std::endl;

    //    utl::erase_duplicates(candidates);
    //    total_certain_ir_exclusions_per_ir_no_doubles += candidates.size();
  }

  std::cout << "Avg sr exclusions per ir: "
            << total_sr_exclusions_per_ir / irs.interlocking_routes_.size()
            << std::endl;
  std::cout << "Avg ir rxclusion candidates per ir: "
            << total_ir_exclusion_candidates_per_ir /
                   irs.interlocking_routes_.size()
            << std::endl;
  std::cout << "Avg certian ir rxclusion per ir: "
            << total_certain_ir_exclusions_per_ir /
                   irs.interlocking_routes_.size()
            << std::endl;
  std::cout << "Avg ir rxclusion candidates per ir (no doubles): "
            << total_ir_exclusion_candidates_per_ir_no_doubles /
                   irs.interlocking_routes_.size()
            << std::endl;
  std::cout << "Avg certian ir rxclusion per ir (no doubles): "
            << total_certain_ir_exclusions_per_ir_no_doubles /
                   irs.interlocking_routes_.size()
            << std::endl;

  return get_interlocking_route_exclusions(irs, station_route_exclusions,
                                           infra);

  //  auto const sorted_exclusion_elements =
  //      soro::to_vec(ssr_m.interlocking_routes_, [&](auto&& ssr) {
  //        auto elements = get_exclusion_elements(*ssr, iss);
  //        utls::sort(elements);
  //        return elements;
  //      });
  //
  //  auto const conflicts = [&](ir_ptr ssr1, ir_ptr ssr2) {};
  //
  //  return soro::to_vec(ssr_m.interlocking_routes_, [&](auto&& ssr) {
  //    soro::vector<ir_ptr> result = get_conflict_candidates(ssr, ssr_m);
  //
  //    utl::erase_if(result,
  //                  [&](auto&& candidate) { return !conflicts(ssr, candidate);
  //                  });
  //
  //    return result;
  //  });
}

}  // namespace soro::infra
