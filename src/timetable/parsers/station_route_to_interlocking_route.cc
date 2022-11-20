#include "soro/timetable/parsers/station_route_to_interlocking_route.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/logging.h"

#include "soro/utls/algo/sort_and_intersect.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

// auto get_best_fitting_ssr2(soro::vector<ir_ptr> const& candidates,
//                            soro::vector<station_route::ptr> const& sr_run,
//                            soro::size_type current_sr_idx,
//                            infrastructure const& infra) {
//   ir_ptr chosen_ssr = nullptr;
//   soro::size_type prefix_length = 0;
//
//   for (auto const& candidate : candidates) {
//     soro::size_type sr_offset = 0;
//
//     while (sr_offset < candidate->station_routes_.size() &&
//            current_sr_idx + sr_offset < sr_run.size() &&
//            sr_run[current_sr_idx + sr_offset]->id_ ==
//                candidate->sr(sr_offset, infra)->id_) {
//       ++sr_offset;
//     }
//
//     if (sr_offset > prefix_length) {
//       chosen_ssr = candidate;
//       prefix_length = sr_offset;
//     }
//   }
//
//   current_sr_idx += prefix_length - 1;
//
//   return std::pair(current_sr_idx, chosen_ssr);
// }
//

interlocking_route::id get_first_ir(
    soro::vector<station_route::ptr> const& sr_run,
    soro::vector<stop> const& stops, FreightTrain const freight,
    interlocking_subsystem const& irs, station_route_graph const&, bool const) {

  auto const first_sr = sr_run.front();

  // make a copy, we will modify the contents
  auto candidates = irs.sr_to_participating_irs_[first_sr->id_];

  if (stops.front().is_halt()) {
    auto const halt = first_sr->get_halt_node(freight);

    if (!halt.has_value()) {
      //      uLOG(utl::warn) << "Got halt in stops, but corresponding station
      //      route "
      //                      << first_sr->name_ << " in station "
      //                      << first_sr->station_->ds100_ << " does not have a
      //                      halt.";
      return interlocking_route::INVALID;
    }

    utl::concat(candidates, irs.halting_at_.at(halt.value()->id_));
  }

  if (candidates.size() == 1) {
    return candidates.front();
  }

  if (stops.front().is_halt()) {
    utl::erase_duplicates(candidates);
  }

  if (candidates.size() == 1) {
    return candidates.front();
  }

  //  std::cout << "candidates size: " << candidates.size() << '\n';

  return 0;
}

// auto get_first_ssr2(soro::vector<station_route::ptr> const& sr_run,
//                     FreightTrain const freight,
//                     interlocking_subsystem const& ssr_man,
//                     station_route_graph const& srg, bool const,
//                     infrastructure const& infra) {
//   auto const& first_sr = sr_run.front();
//
//   auto parti_candidates = ssr_man.sr_to_participating_irs_[first_sr->id_];
//   auto stop_candidates =
//       ssr_man.halting_at_.at(sr_run.front()->get_halt_node(freight)->id_);
//
//   auto const& candidates =
//       utls::sort_and_intersect(parti_candidates, stop_candidates);
//
//   if (srg.predeccesors_[first_sr->id_].empty()) {
//     return get_best_fitting_ssr2(candidates, sr_run, 0, infra);
//   }
//
//   ir_ptr chosen_ssr = nullptr;
//
//   soro::size_type prefix_length = 0;
//
//   for (auto const& candidate : candidates) {
//     soro::size_type sr_offset = 0;
//
//     auto parti_it = utls::find(candidate->station_routes_, first_sr);
//
//     while (parti_it != std::end(candidate->station_routes_) &&
//            (sr_run[0 + sr_offset]->id_ == (*parti_it)->id_)) {
//
//       ++sr_offset;
//       std::advance(parti_it, 1);
//     }
//
//     if (sr_offset > prefix_length) {
//       chosen_ssr = candidate;
//       prefix_length = sr_offset;
//     }
//   }
//
//   return std::pair(prefix_length - 1, chosen_ssr);
// }

soro::vector<ir_ptr> get_interlocking_route_path(
    raw_train::run const& run, FreightTrain const freight,
    interlocking_subsystem const& irs, station_route_graph const& srg) {
  //  soro::vector<ir_ptr> ssr_path;

  auto const first_ir =
      get_first_ir(run.routes_, run.stops_, freight, irs, srg, run.break_in_);

  if (!interlocking_route::valid(first_ir)) {
    return {};
  }

  return {nullptr};
}

//  auto [current_sr_idx, chosen_ssr] =
//      get_first_ssr2(run.routes_, freight, ssr_man, srg, run.break_in_);
//  ssr_path.emplace_back(chosen_ssr);
//  node_ptr current_node = ssr_path.back()->nodes().back();
//
//  while (current_sr_idx != run.routes_.size() - 1) {
//
//    // TODO(julian) this should become unnecessary when all train paths
//    // contained in a timetable are parsed correctly
//    auto const can_it = ssr_man.starting_at_.find(current_node->id_);
//    if (can_it == std::end(ssr_man.starting_at_)) {
//      ssr_path.clear();
//      return ssr_path;
//    }
//
//    auto const& candidates = can_it->second;
//
//    std::tie(current_sr_idx, chosen_ssr) =
//        get_best_fitting_ssr2(candidates, run.routes_, current_sr_idx);
//
//    // TODO(julian) this should become unnecessary when all train paths
//    // contained in a timetable are parsed correctly
//    if (chosen_ssr == nullptr) {
//      ssr_path.clear();
//      return ssr_path;
//    }
//
//    ssr_path.emplace_back(chosen_ssr);
//    current_node = ssr_path.back()->nodes().back();
//  }
//
//  auto const& last_sr = run.routes_[current_sr_idx];
//
//  while (true) {
//    auto const& candidate_it = ssr_man.starting_at_.find(current_node->id_);
//
//    if (candidate_it == std::cend(ssr_man.starting_at_)) {
//      break;
//    }
//
//    auto chosen_it = utls::find_if(candidate_it->second, [&](ir_ptr can) {
//      return can->station_routes_.front()->id_ == last_sr->id_;
//    });
//
//    if (chosen_it == std::end(candidate_it->second)) {
//      break;
//    }
//
//    chosen_ssr = *chosen_it;
//
//    bool const found_internal = chosen_ssr->station_routes_.size() == 1;
//    ssr_path.emplace_back(chosen_ssr);
//
//    if (!found_internal) {
//      break;
//    }
//
//    current_node = ssr_path.back()->nodes().back();
//  }
//
//  return ssr_path;

}  // namespace soro::tt