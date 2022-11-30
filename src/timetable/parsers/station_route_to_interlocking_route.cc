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
                       stop_sequence const& stop_sequence,
                       std::size_t const sr_offset) {
  auto sr1 = interlocking_route.station_routes_.begin();
  auto sr2 =
      std::begin(stop_sequence.points_) + static_cast<ssize_t>(sr_offset);

  std::size_t prefix_length = 0;

  for (;
       sr1 != std::end(interlocking_route.station_routes_) &&
       sr2 != std::end(stop_sequence.points_) && *sr1 == (*sr2).station_route_;
       ++sr1, ++sr2) {
    ++prefix_length;
  }

  return prefix_length;
}

auto get_longest_prefix_interlocking_routes(
    std::vector<interlocking_route::id> const& candidates,
    interlocking_subsystem const& irs, stop_sequence const& stop_sequence,
    std::size_t const sr_offset) {

  std::size_t current_longset_prefix = 0;
  std::vector<interlocking_route::id> longest_prefixes;

  for (auto const candidate : candidates) {
    auto const& candidate_route = irs.interlocking_routes_[candidate];
    auto const prefix_length =
        get_prefix_length(candidate_route, stop_sequence, sr_offset);

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
// template <class ForwardIterable, class GetKey>
// constexpr auto max_element(ForwardIterable&& fw_iterable, GetKey&& get_key)
//    -> std::pair<
//        decltype(std::begin(fw_iterable)),
//        std::invoke_result_t<GetKey, decltype(*std::begin(fw_iterable))>> {
//  auto first = std::begin(fw_iterable);
//  auto last = std::end(fw_iterable);
//
//  if (first == last) {
//    return {last, {}};
//  }
//
//  auto largest_element = first;
//  auto largest_key = get_key(*first);
//
//  ++first;
//  for (; first != last; ++first) {
//    auto const key_largest = get_key(*largest_element);
//    auto const key_first = get_key(*first);
//
//    if (key_largest < key_first) {
//      largest_element = first;
//      largest_key = key_largest;
//    }
//  }
//
//  return {largest_element, largest_key};
//}

std::pair<interlocking_route::id, std::size_t> get_first_ir(
    stop_sequence const& stop_sequence, FreightTrain const freight,
    infrastructure const& infra) {
  auto const& irs = infra->interlocking_;

  interlocking_route::id ir_result = interlocking_route::INVALID;
  std::size_t run_offset_result = std::numeric_limits<std::size_t>::max();

  auto const first_sr =
      infra->station_routes_[stop_sequence.points_.front().station_route_];

  // make a copy, we will modify the contents
  auto candidates = irs.sr_to_participating_irs_[first_sr->id_];

  if (stop_sequence.points_.front().is_halt()) {
    auto const halt = first_sr->get_halt_node(freight);

    if (!halt.has_value()) {
      ++failures[failure_reason::halt_in_stop_but_none_in_station_route];
      return {ir_result, run_offset_result};
    }

    utl::concat(candidates, irs.halting_at_.at(halt.value()->id_));
  }

  if (stop_sequence.points_.front().is_halt()) {
    utl::erase_duplicates(candidates);
  }

  auto const [longest_prefix_routes, current_longest_prefix] =
      get_longest_prefix_interlocking_routes(candidates, irs, stop_sequence, 0);

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

// void take(stop_sequence const& stop_sequence, std::size_t const from,
// std::size_t const to,
//           interlocking_route::id const ir_id) {
//
// }

utls::optional<infra::node::idx> get_node_idx(
    sequence_point const& sp, rs::FreightTrain const freight,
    infra::infrastructure const& infra) {
  auto const sr = infra->station_routes_[sp.station_route_];

  if (sp.has_transit_time()) {
    return sr->runtime_checkpoint_;
  }

  if (sp.is_halt()) {
    return sr->get_halt_idx(freight);
  }

  return {};
}

soro::vector<sequence_point> get_sequence_points(interlocking_route const& ir,
                                                 stop_sequence const& ss,
                                                 rs::FreightTrain const freight,
                                                 std::size_t const last_prefix,
                                                 std::size_t const new_prefix,
                                                 infrastructure const& infra) {
  soro::vector<sequence_point> result;

  for (auto sp_idx = last_prefix; sp_idx < new_prefix; ++sp_idx) {
    auto const sequence_point = ss.points_[sp_idx];

    auto const sr_node_idx = get_node_idx(sequence_point, freight, infra);

    if (sr_node_idx.has_value() &&
        ir.contains(sequence_point.station_route_, *sr_node_idx)) {
      result.emplace_back(sequence_point);
    }
  }

  return result;
}

struct cover {

  bool covers(struct stop_sequence const& ss, rs::FreightTrain const freight,
              infrastructure const& infra) const {
    if (stop_sequence_offset_ < ss.points_.size() - 1) {
      return false;
    }

    auto const last_sr =
        infra->station_routes_[ss.points_.back().station_route_];

    node::idx last_stop_sequence_node = node::INVALID_IDX;

    if (ss.points_.back().is_halt()) {
      last_stop_sequence_node = last_sr->get_halt_idx(freight).value() + 1;
    } else if (ss.break_out_) {
      last_stop_sequence_node = last_sr->size();
    }

    return node_idx_ > last_stop_sequence_node;
  }

  std::size_t stop_sequence_offset_{0};
  std::size_t node_idx_{0};
};

train::path get_interlocking_route_path(stop_sequence const& stop_sequence,
                                        FreightTrain const freight,
                                        infrastructure const& infra) {
  auto const& irs = infra->interlocking_;

  std::cout << "Train\n";

  auto [first_ir_id, initial_sr_offset] =
      get_first_ir(stop_sequence, freight, infra);

  if (!interlocking_route::valid(first_ir_id)) {
    return {};
  }

  train::path train_path;
  train_path.break_in_ = stop_sequence.break_in_;
  train_path.break_out_ = stop_sequence.break_out_;

  interlocking_route::ptr current_ir = &irs.interlocking_routes_[first_ir_id];

  cover ss_cover{.stop_sequence_offset_ = initial_sr_offset,
                 .node_idx_ = current_ir->end_offset_};

  train::path::entry first_tpe;
  first_tpe.interlocking_id_ = first_ir_id;
  first_tpe.sequence_points_ =
      get_sequence_points(*current_ir, stop_sequence, freight, 0,
                          ss_cover.stop_sequence_offset_ + 1, infra);
  train_path.entries_.emplace_back(first_tpe);

  while (!ss_cover.covers(stop_sequence, freight, infra)) {
    //  while (current_sr_offset < stop_sequence.points_.size() - 1) {
    auto const& candidates =
        irs.starting_at_[current_ir->last_node(infra)->id_];

    auto const [longest_prefix_routes, prefix_length] =
        get_longest_prefix_interlocking_routes(candidates, irs, stop_sequence,
                                               ss_cover.stop_sequence_offset_);

    if (longest_prefix_routes.size() > 1) {
      ++failures[failure_reason::could_not_find_unique_ir];
      return {};
    }

    if (longest_prefix_routes.empty()) {
      ++failures[failure_reason::could_not_find_ir];
      return {};
    }

    current_ir = &irs.interlocking_routes_[longest_prefix_routes.front()];

    train::path::entry tpe;
    tpe.interlocking_id_ = current_ir->id_;
    tpe.sequence_points_ = get_sequence_points(
        *current_ir, stop_sequence, freight, ss_cover.stop_sequence_offset_,
        ss_cover.stop_sequence_offset_ + prefix_length, infra);
    train_path.entries_.emplace_back(tpe);

    std::cout << "current sr offset: " << ss_cover.stop_sequence_offset_
              << std::endl;
    std::cout << "prefix length: " << prefix_length << std::endl;

    // true when the current interlocking route and the currently worked on
    // station route end on the same node
    auto const last_sr_id =
        stop_sequence.points_[ss_cover.stop_sequence_offset_].station_route_;
    auto const ends_align = current_ir->last_node(infra) ==
                            infra->station_routes_[last_sr_id]->nodes().back();

    // when the ends align we have completely inserted the current station route
    // into the interlocking path, so we can move on to the next station route
    ss_cover.stop_sequence_offset_ +=
        ends_align ? prefix_length : prefix_length - 1;
    ss_cover.node_idx_ = current_ir->end_offset_;
  }

  //  node::idx last_sr_node_offset = node::INVALID_IDX;
  //
  //  auto const completely_covered = [](struct stop_sequence const& ss) {
  //    return ss.points_.size()
  //  };
  //
  //  if (stop_sequence.break_out_) {
  //    last_sr_node_offset =
  //        infra->station_routes_[stop_sequence.points_.back().station_route_]
  //            ->size();
  //  } else {
  //    last_sr_node_offset =
  //        infra->station_routes_[stop_sequence.points_.back().station_route_]
  //            ->get_halt_idx(freight)
  //            .value();
  //  }

  //  // we have to
  //  if (last_sr_node_offset != current_ir->end_offset_) {
  //
  //    auto const& candidates =
  //        irs.starting_at_[current_ir->last_node(infra)->id_];
  //  }

  utls::sasserts([&]() {
    auto const measurable_points = utls::count_if(
        stop_sequence.points_, [](auto&& sp) { return sp.is_measurable(); });

    auto const path_points = utls::accumulate(
        train_path.entries_, std::size_t{0}, [](auto&& acc, auto&& tpe) {
          return acc + tpe.sequence_points_.size();
        });

    utls::sassert(measurable_points == path_points);
  });

  return train_path;
}

}  // namespace soro::tt