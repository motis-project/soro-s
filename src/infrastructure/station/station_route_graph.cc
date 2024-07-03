#include "soro/infrastructure/station/station_route_graph.h"

#include <iterator>

#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/std_wrapper/transform.h"

#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

auto get_successors_from_through_route(station_route::ptr sr) {
  bool const ends_in_track_end = sr->nodes().back()->is(type::TRACK_END);
  if (ends_in_track_end || !sr->to_station_.has_value()) {
    return soro::vector<station_route::ptr>();
  }

  auto const& to_border = sr->nodes().back()->next_->element_;

  if (auto it =
          sr->to_station_.value()->element_to_routes_.find(to_border->get_id());
      it != std::end(sr->to_station_.value()->element_to_routes_)) {
    return it->second;
  } else {
    return soro::vector<station_route::ptr>();
  }
}

auto get_successors_from_in_route(station_route::ptr, graph const&) {
  // TODO(julian) whats with this?
  soro::vector<station_route::ptr> succs;
  return succs;

  //  auto const& last_element = sr->nodes().back()->element_;
  //  auto const section_ids =
  //      network.element_id_to_section_ids_[last_element->id()];
  //
  //  utl::verify(section_ids.size() == 1,
  //              "There should only be a single section id associated with the
  //              " "last element!");
  //
  //  auto const section = network.sections_[section_ids.front()];
  //
  //  for (auto const& element_ptr : section.elements_) {
  //    if (!element_ptr->is(type::HALT)) {
  //      continue;
  //    }
  //
  //    auto it = sr->station_->element_to_routes_.find(element_ptr->id());
  //    if (it == std::cend(sr->station_->element_to_routes_)) {
  //      continue;
  //    }
  //
  //    for (auto candidate : it->second) {
  //      // only add the candidate as a successor if:
  //      // it has the same orientation as the station route
  //      bool const same_orientation =
  //          candidate->nodes().front()->element_->rising() ==
  //          sr->nodes().back()->element_->rising();
  //
  //      // and there is no backward driving, like in this example
  //      // the last of the station route to exclude the following case:
  //      // O - - HLT - - MS - - HLT - - O
  //      // 1 ------------------->
  //      // 2      ---------------------->
  //      // chaining of 1 -> 2, which would yield a signal station route which
  //      // starts and ends at the same MS
  //      auto const distance =
  //          sr->nodes().back()->element_->as<track_element>().km_ -
  //          candidate->nodes().front()->element_->as<track_element>().km_;
  //
  //      bool const no_backward_driving =
  //      sr->nodes().back()->element_->rising()
  //                                           ? distance <=
  //                                           si::ZERO<kilometrage> : distance
  //                                           >= si::ZERO<kilometrage>;
  //
  //      if (same_orientation && no_backward_driving) {
  //        succs.push_back(candidate);
  //      }
  //    }
  //  }
  //
  //  return succs;
}

auto get_successors_from_out_route(station_route::ptr sr) {
  return get_successors_from_through_route(sr);
}

auto get_successors_from_half_route(station_route::ptr sr,
                                    graph const& network) {
  if (sr->is_in_route()) {
    return get_successors_from_in_route(sr, network);
  } else {
    return get_successors_from_out_route(sr);
  }
}

soro::vector<station_route::ptr> get_successors(station_route::ptr sr,
                                                graph const& network) {
  if (sr->is_through_route()) {
    return get_successors_from_through_route(sr);
  } else {
    return get_successors_from_half_route(sr, network);
  }
}

station_route_graph get_station_route_graph(
    soro::vector_map<station_route::id, station_route::ptr> const&
        station_routes,
    graph const& network) {
  utl::scoped_timer const srg_timer("Building Station Route Graph");

  station_route_graph srg;

  srg.successors_.reserve(station_routes.size());
  utls::transform(station_routes, std::back_inserter(srg.successors_),
                  [&](auto&& sr) { return get_successors(sr, network); });

  srg.predeccesors_.resize(station_routes.size());
  for (auto const& sr : station_routes) {
    auto const& succs = srg.successors_[sr->id_];
    for (auto const& succ : succs) {
      srg.predeccesors_[succ->id_].emplace_back(sr);
    }
  }

  return srg;
}

}  // namespace soro::infra
