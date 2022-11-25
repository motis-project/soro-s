#include "soro/timetable/parsers/station_route_to_interlocking_route.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/logging.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

struct failures {};

enum class failure_reason : uint8_t {
  could_not_find_first_ir,
  could_not_find_unique_first_ir,
  could_not_find_ir,
  could_not_find_unique_ir,
  halt_in_stop_but_none_in_station_route
};

static std::map<failure_reason, std::size_t> failures = {  // NOLINT
    {failure_reason::could_not_find_first_ir, 0},
    {failure_reason::could_not_find_unique_first_ir, 0},
    {failure_reason::could_not_find_ir, 0},
    {failure_reason::could_not_find_unique_ir, 0},
    {failure_reason::halt_in_stop_but_none_in_station_route, 0}};

static std::map<failure_reason, std::string> failure_text = {  // NOLINT
    {failure_reason::could_not_find_first_ir,
     "Could not find any first interlocking route: "},
    {failure_reason::could_not_find_unique_first_ir,
     "Could not find unique first interlocking route: "},
    {failure_reason::could_not_find_ir,
     "Could not find any interlocking route: "},
    {failure_reason::could_not_find_unique_ir,
     "Could not find unique interlocking route: "},
    {failure_reason::halt_in_stop_but_none_in_station_route,
     "Halt in stops but no halt in station route: "}};

void print_ir_generating_failures() {
  for (auto const& [reason, count] : failures) {
    uLOG(utl::warn) << failure_text[reason] << count;
  }
}

auto get_prefix_length(interlocking_route const& interlocking_route,
                       soro::vector<station_route::ptr> const& sr_run,
                       std::size_t const sr_offset) {
  auto sr1 = interlocking_route.station_routes_.begin();
  auto sr2 = std::begin(sr_run) + static_cast<ssize_t>(sr_offset);

  std::size_t prefix_length = 0;

  for (; sr1 != std::end(interlocking_route.station_routes_) &&
         sr2 != std::end(sr_run) && *sr1 == (*sr2)->id_;
       ++sr1, ++sr2) {
    ++prefix_length;
  }

  return prefix_length;
}

auto get_longest_prefix_interlocking_routes(
    std::vector<interlocking_route::id> const& candidates,
    interlocking_subsystem const& irs,
    soro::vector<station_route::ptr> const& sr_run,
    std::size_t const sr_offset) {

  std::size_t current_longset_prefix = 0;
  std::vector<interlocking_route::id> longest_prefixes;

  for (auto const candidate : candidates) {
    auto const& candidate_route = irs.interlocking_routes_[candidate];
    auto const prefix_length =
        get_prefix_length(candidate_route, sr_run, sr_offset);

    if (prefix_length > current_longset_prefix) {
      longest_prefixes.clear();
      current_longset_prefix = prefix_length;
    }

    if (prefix_length >= current_longset_prefix) {
      longest_prefixes.emplace_back(candidate);
    }
  }

  return std::pair{longest_prefixes, current_longset_prefix};
}

// TODO(julian) move to utls and write test || remove this
template <class ForwardIterable, class GetKey>
constexpr auto max_element(ForwardIterable&& fw_iterable, GetKey&& get_key)
    -> std::pair<
        decltype(std::begin(fw_iterable)),
        std::invoke_result_t<GetKey, decltype(*std::begin(fw_iterable))>> {
  auto first = std::begin(fw_iterable);
  auto last = std::end(fw_iterable);

  if (first == last) {
    return {last, {}};
  }

  auto largest_element = first;
  auto largest_key = get_key(*first);

  ++first;
  for (; first != last; ++first) {
    auto const key_largest = get_key(*largest_element);
    auto const key_first = get_key(*first);

    if (key_largest < key_first) {
      largest_element = first;
      largest_key = key_largest;
    }
  }

  return {largest_element, largest_key};
}

std::pair<interlocking_route::id, std::size_t> get_first_ir(
    soro::vector<station_route::ptr> const& sr_run,
    soro::vector<stop> const& stops, FreightTrain const freight,
    interlocking_subsystem const& irs, station_route_graph const&, bool const) {

  interlocking_route::id ir_result = interlocking_route::INVALID;
  std::size_t run_offset_result = std::numeric_limits<std::size_t>::max();

  auto const first_sr = sr_run.front();

  // make a copy, we will modify the contents
  auto candidates = irs.sr_to_participating_irs_[first_sr->id_];

  if (stops.front().is_halt()) {
    auto const halt = first_sr->get_halt_node(freight);

    if (!halt.has_value()) {
      ++failures[failure_reason::halt_in_stop_but_none_in_station_route];
      return {ir_result, run_offset_result};
    }

    utl::concat(candidates, irs.halting_at_.at(halt.value()->id_));
  }

  if (stops.front().is_halt()) {
    utl::erase_duplicates(candidates);
  }

  auto const [longest_prefix_routes, current_longest_prefix] =
      get_longest_prefix_interlocking_routes(candidates, irs, sr_run, 0);

  if (longest_prefix_routes.size() == 1) {
    return {longest_prefix_routes.front(), current_longest_prefix - 1};
  }

  if (longest_prefix_routes.size() > 1) {
    ++failures[failure_reason::could_not_find_unique_first_ir];
  }

  if (longest_prefix_routes.empty()) {
    ++failures[failure_reason::could_not_find_first_ir];
  }

  return {interlocking_route::INVALID, 1000};
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

soro::vector<interlocking_route::id> get_interlocking_route_path(
    raw_train::run const& run, FreightTrain const freight,
    infrastructure const& infra) {
  auto const& irs = infra->interlocking_;
  auto const& srg = infra->station_route_graph_;

  auto [first_ir, current_sr_offset] =
      get_first_ir(run.routes_, run.stops_, freight, irs, srg, run.break_in_);

  if (!interlocking_route::valid(first_ir)) {
    return {};
  }

  soro::vector<interlocking_route::id> ir_path = {first_ir};

  interlocking_route::ptr current_ir = &irs.interlocking_routes_[first_ir];

  while (current_sr_offset < run.routes_.size() - 1) {
    auto const& candidates =
        irs.starting_at_[current_ir->last_node(infra)->id_];

    auto const [longest_prefix_routes, prefix_length] =
        get_longest_prefix_interlocking_routes(candidates, irs, run.routes_,
                                               current_sr_offset);

    if (longest_prefix_routes.size() == 1) {
      ir_path.emplace_back(longest_prefix_routes.front());
    }

    if (longest_prefix_routes.size() > 1) {
      ++failures[failure_reason::could_not_find_unique_ir];
      return {};
    }

    if (longest_prefix_routes.empty()) {
      ++failures[failure_reason::could_not_find_ir];
      return {};
    }

    current_ir = &irs.interlocking_routes_[ir_path.back()];

    // true when the current interlocking route and the currently worked on
    // station route end on the same node
    auto const ends_align = current_ir->last_node(infra) ==
                            run.routes_[current_sr_offset]->nodes().back();

    // when the ends align we have completely inserted the current station route
    // into the interlocking path, so we can move on to the next station route
    current_sr_offset += ends_align ? prefix_length : prefix_length - 1;
  }

  return ir_path;
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