#include "soro/timetable/parsers/station_route_to_interlocking_route.h"

#include "soro/utls/result.h"
#include "utl/concat.h"
#include "utl/logging.h"

#include "soro/utls/algo/sort_and_intersect.h"
#include "soro/utls/std_wrapper/count_if.h"
#include "soro/utls/std_wrapper/find_position.h"

#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/timetable/parsers/route_transform_error.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

auto get_prefix_length(interlocking_route const& interlocking_route,
                       stop_sequence const& stop_sequence,
                       std::size_t const sr_offset) {
  auto sr1 = interlocking_route.station_routes_.begin();
  auto sr2 = std::begin(stop_sequence.points_) +
             static_cast<std::ptrdiff_t>(sr_offset);

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
    soro::vector<interlocking_route::id> const& candidates,
    interlocking const& irs, stop_sequence const& stop_sequence,
    std::size_t const sr_offset) {

  std::size_t current_longset_prefix = 0;
  std::vector<interlocking_route::id> longest_prefixes;

  for (auto const candidate : candidates) {
    auto const& candidate_route = irs.routes_[candidate];
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

// this holds information about how much of a stop sequence is covered
struct cover {
  bool covers(stop_sequence const& ss, rs::FreightTrain const freight,
              infrastructure const& infra) const {
    if (stop_sequence_offset_ < ss.points_.size() - 1) {
      return false;
    }

    auto const last_sr =
        infra->station_routes_[ss.points_.back().station_route_];

    node::idx last_stop_sequence_node = node::INVALID_IDX;

    if (ss.points_.back().is_halt()) {
      last_stop_sequence_node = last_sr->get_halt_idx(freight).value();
    } else if (ss.break_out_) {
      last_stop_sequence_node = last_sr->size() - 1;
    }

    return node_idx_ > last_stop_sequence_node;
  }

  std::size_t stop_sequence_offset_{0};
  std::size_t node_idx_{0};
};

// for a given stop sequence and a given current cover, find all possible next
// interlocking routes
interlocking_route::ids get_candidates(stop_sequence const& ss,
                                       cover const& current_cover,
                                       node::optional_ptr const& last_node,
                                       rs::FreightTrain const freight,
                                       infrastructure const& infra) {
  auto const& sp = ss.points_[current_cover.stop_sequence_offset_];
  auto candidates =
      infra->interlocking_.sr_to_participating_irs_[sp.station_route_];

  soro::vector<interlocking_route::id> const* starting_candidates = nullptr;
  if (last_node.has_value()) {
    starting_candidates = &infra->interlocking_.starting_at_[(*last_node)->id_];
    candidates = utls::intersect(candidates, *starting_candidates);
  }

  if (candidates.size() == 1) {
    return candidates;
  }

  soro::vector<interlocking_route::id> const* halt_candidates = nullptr;
  if (sp.is_halt() &&
      current_cover.node_idx_ <= *sp.get_node_idx(freight, infra)) {
    auto halt_node = *(sp.get_node(freight, infra));
    halt_candidates = &infra->interlocking_.halting_at_[halt_node->id_];
    candidates = utls::intersect(candidates, *halt_candidates);
  }

  return candidates;
}

struct first_ir {
  interlocking_route::id id_;
  cover cover_;
};

utls::result<first_ir> get_first_ir(stop_sequence const& stop_sequence,
                                    FreightTrain const freight,
                                    infrastructure const& infra) {
  auto const& irs = infra->interlocking_;

  cover ss_cover{.stop_sequence_offset_ = 0, .node_idx_ = 0};

  auto const first_sr =
      infra->station_routes_[stop_sequence.points_.front().station_route_];

  auto const first_node = stop_sequence.break_in_
                              ? node::optional_ptr{first_sr->nodes().front()}
                              : std::nullopt;

  auto const candidates =
      get_candidates(stop_sequence, ss_cover, first_node, freight, infra);

  auto const [longest_prefix_routes, current_longest_prefix] =
      get_longest_prefix_interlocking_routes(candidates, irs, stop_sequence, 0);

  if (longest_prefix_routes.empty()) {
    return std::unexpected(error::route_transform::NO_FIRST_IR);
  }

  if (longest_prefix_routes.size() > 1) {
    return std::unexpected(error::route_transform::NO_UNIQUE_FIRST_IR);
  }

  auto const& prefix_ir = irs.routes_[longest_prefix_routes.front()];

  ss_cover.node_idx_ = prefix_ir.end_offset_;
  if (current_longest_prefix == 0) {
    ss_cover.stop_sequence_offset_ =
        prefix_ir.station_routes_.size() -
        utls::find_position(prefix_ir.station_routes_, first_sr->id_) - 1;
  } else {
    ss_cover.stop_sequence_offset_ = current_longest_prefix - 1;
  }

  if (longest_prefix_routes.size() == 1) {
    return first_ir{.id_ = longest_prefix_routes.front(), .cover_ = ss_cover};
  }

  return std::unexpected(error::route_transform::NO_UNIQUE_FIRST_IR);
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

    auto const sr_node_idx = sequence_point.get_node_idx(freight, infra);

    if (sr_node_idx.has_value() &&
        ir.contains(sequence_point.station_route_, *sr_node_idx)) {
      result.emplace_back(sequence_point);
    }
  }

  return result;
}

utls::result<interlocking_transformation> transform_to_interlocking(
    stop_sequence const& stop_sequence, FreightTrain const freight,
    infrastructure const& infra) {
  interlocking_transformation result;

  auto const& irs = infra->interlocking_;

  auto const first_ir = get_first_ir(stop_sequence, freight, infra);

  if (!first_ir) {
    return utls::propagate(first_ir);
  }

  auto current_cover = first_ir->cover_;

  interlocking_route::ptr current_ir = &irs.routes_[first_ir->id_];

  result.path_.emplace_back(first_ir->id_);
  utl::concat(
      result.sequence_points_,
      get_sequence_points(*current_ir, stop_sequence, freight, 0,
                          current_cover.stop_sequence_offset_ + 1, infra));

  while (!current_cover.covers(stop_sequence, freight, infra)) {
    auto const candidates = get_candidates(
        stop_sequence, current_cover,
        node::optional_ptr(current_ir->last_node(infra)), freight, infra);

    auto const [longest_prefix_routes, prefix_length] =
        get_longest_prefix_interlocking_routes(
            candidates, irs, stop_sequence,
            current_cover.stop_sequence_offset_);

    if (longest_prefix_routes.size() > 1) {
      return std::unexpected(error::route_transform::NO_UNIQUE_IR);
    }

    if (longest_prefix_routes.empty()) {
      return std::unexpected(error::route_transform::NO_IR);
    }

    current_ir = &irs.routes_[longest_prefix_routes.front()];

    result.path_.emplace_back(current_ir->id_);
    utl::concat(
        result.sequence_points_,
        get_sequence_points(*current_ir, stop_sequence, freight,
                            current_cover.stop_sequence_offset_,
                            current_cover.stop_sequence_offset_ + prefix_length,
                            infra));

    // the current cover + the current interlocking route cover more
    // than the given stop sequence.
    // thus, it is fully covered
    if (current_cover.stop_sequence_offset_ +
            current_ir->station_routes_.size() >
        stop_sequence.points_.size()) {
      break;
    }

    // true when the current interlocking route and the currently worked on
    // station route end on the same node
    auto const last_sr_id =
        stop_sequence.points_[current_cover.stop_sequence_offset_]
            .station_route_;
    auto const ends_align = current_ir->last_node(infra) ==
                            infra->station_routes_[last_sr_id]->nodes().back();

    // when the ends align we have completely inserted the current station route
    // into the interlocking path, so we can move on to the next station route
    current_cover.stop_sequence_offset_ +=
        ends_align ? prefix_length : prefix_length - 1;
    current_cover.node_idx_ = current_ir->end_offset_;
  }

  utls::ensures([&] {
    auto const measurable_points = utls::count_if(
        stop_sequence.points_, [](auto&& sp) { return sp.is_measurable(); });

    utls::ensure(measurable_points == result.sequence_points_.size());
  });

  return result;
}

}  // namespace soro::tt