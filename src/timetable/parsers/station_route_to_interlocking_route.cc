#include "soro/timetable/parsers/station_route_to_interlocking_route.h"

#include <algorithm>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

#include "soro/base/soro_types.h"

#include "soro/utls/algo/sort_and_intersect.h"
#include "soro/utls/narrow.h"
#include "soro/utls/result.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/distance.h"
#include "soro/utls/std_wrapper/find_position.h"
#include "soro/utls/std_wrapper/min_element.h"

#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/interlocking/interlocking.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/timetable/parsers/route_transform_error.h"
#include "soro/timetable/sequence_point.h"
#include "soro/timetable/train.h"

namespace soro::tt {

using namespace soro::rs;
using namespace soro::infra;

soro::size_t get_prefix_length(
    interlocking_route const& interlocking_route,
    soro::vector<sequence_point> const& sequence_points,
    soro::size_t const sr_offset) {
  auto const ir_does_not_contain_sr = [&](auto const& sp) {
    return !utls::contains(interlocking_route.station_routes_,
                           sp.station_route_);
  };

  auto const from_it = std::begin(sequence_points) + sr_offset;
  auto const to_it =
      std::find_if(from_it, std::end(sequence_points), ir_does_not_contain_sr);

  auto const prefix_length = utls::distance<soro::size_t>(from_it, to_it);

  return prefix_length;
}

auto get_longest_prefix_interlocking_routes(
    soro::vector<interlocking_route::id> const& candidates,
    interlocking const& irs,
    soro::vector<sequence_point> const& sequence_points,
    soro::size_t const sr_offset) {

  soro::size_t current_longest_prefix = 0;
  soro::vector<interlocking_route::id> longest_prefixes;

  for (auto const candidate : candidates) {
    auto const& candidate_route = irs.routes_[candidate];
    auto const prefix_length =
        get_prefix_length(candidate_route, sequence_points, sr_offset);

    if (prefix_length > current_longest_prefix) {
      longest_prefixes.clear();
      current_longest_prefix = prefix_length;
    }

    if (prefix_length >= current_longest_prefix) {
      longest_prefixes.emplace_back(candidate);
    }
  }

  return std::pair{longest_prefixes, current_longest_prefix};
}

// this holds information about how much of a stop sequence is covered
struct cover {
  bool covers(train const& train, infrastructure const& infra) const {
    if (stop_sequence_offset_ < train.sequence_points_.size() - 1) {
      return false;
    }

    // TODO(julian) support train not ending on halt or breaking out
    utls::sassert(train.break_out_ || train.sequence_points_.back().is_halt(),
                  "train does not end on halt");

    auto const last_node_idx = train.break_out_
                                   ? train.last_station_route(infra)->size() - 1
                                   : train.sequence_points_.back().idx_;

    return node_idx_ > last_node_idx;
  }

  soro::size_t stop_sequence_offset_{0};
  station_route::idx node_idx_{0};
};

// for a given stop sequence and a given current cover, find all possible next
// interlocking routes
interlocking_route::ids get_candidates(
    soro::vector<sequence_point> const& sequence_points, cover const& cover,
    node::id const last_node_id, infrastructure const& infra) {
  auto const& sp = sequence_points[cover.stop_sequence_offset_];

  // we will create a set of 'candidate' interlocking routes that can serve as
  // the next interlocking route

  // start by building the set by taking all interlocking routes that use the
  // current station route
  auto candidates = infra->interlocking_.sr_to_irs_[sp.station_route_];

  // intersect the candidates with all interlocking routes starting at the last
  // used node id
  auto const& starting = infra->interlocking_.starting_at_[last_node_id];
  candidates = utls::intersect(candidates, starting);

  if (candidates.size() == 1) return candidates;

  // if the current sequence point is a halt we can intersect the candidate set
  // with all interlocking routes that halt (and reach) the halt node
  if (sp.is_halt() && cover.node_idx_ <= sp.idx_) {
    auto const halt_node = sp.get_node(infra);
    auto const& halting = infra->interlocking_.halting_at_[halt_node->id_];
    candidates = utls::intersect(candidates, halting);
  }

  return candidates;
}

interlocking_route::id get_shortest_route(interlocking_route::ids const& ids,
                                          infrastructure const& infra) {
  utls::expect(!ids.empty(), "empty set of interlocking routes");

  if (ids.size() == 1) return ids.front();

  auto const is_shorter = [&](auto const& ir_id1, auto const& ir_id2) {
    auto const& ir1 = infra->interlocking_.routes_[ir_id1];
    auto const& ir2 = infra->interlocking_.routes_[ir_id2];
    return ir1.length(infra) < ir2.length(infra);
  };

  return *utls::min_element(ids, is_shorter);
}

struct first_ir {
  interlocking_route::id id_;
  cover cover_;
};

utls::result<first_ir> get_first_ir(train const& train,
                                    infrastructure const& infra) {
  auto const& irs = infra->interlocking_;

  cover cover{.stop_sequence_offset_ = 0, .node_idx_ = 0};

  // TODO(julian) support trains with a transit as first sequence point
  utls::sassert(train.sequence_points_.front().is_halt() || train.break_in_,
                "not breaking in and first sp as transit point not supported");

  auto const first_node_id = train.get_start_node(infra)->id_;

  auto const candidates =
      get_candidates(train.sequence_points_, cover, first_node_id, infra);

  auto const [longest_prefix_routes, current_longest_prefix] =
      get_longest_prefix_interlocking_routes(candidates, irs,
                                             train.sequence_points_, 0);

  if (longest_prefix_routes.empty()) {
    return utls::unexpected(error::route_transform::NO_FIRST_IR);
  }

  // if we have multiple first interlocking routes as candidates, we will take
  // the shortest one
  auto const prefix_ir_id = get_shortest_route(longest_prefix_routes, infra);
  auto const& prefix_ir = irs.routes_[prefix_ir_id];

  cover.node_idx_ = prefix_ir.end_offset_;
  if (current_longest_prefix == 0) {
    auto const pos = utls::find_position(prefix_ir.station_routes_,
                                         train.first_station_route_id());
    cover.stop_sequence_offset_ =
        utls::narrow<soro::size_t>(prefix_ir.station_routes_.size() - pos - 1);
  } else {
    cover.stop_sequence_offset_ = current_longest_prefix - 1;
  }

  return first_ir{.id_ = prefix_ir_id, .cover_ = cover};
}

utls::result<soro::vector<interlocking_route::id>> transform_to_interlocking(
    train const& train, infrastructure const& infra) {
  soro::vector<interlocking_route::id> result;

  auto const& irs = infra->interlocking_;

  auto const first_ir = get_first_ir(train, infra);
  if (!first_ir) return utls::propagate(first_ir);

  auto current_cover = first_ir->cover_;

  interlocking_route::ptr current_ir = &irs.routes_[first_ir->id_];

  result.emplace_back(first_ir->id_);
  while (!current_cover.covers(train, infra)) {
    auto const candidates =
        get_candidates(train.sequence_points_, current_cover,
                       current_ir->last_node(infra)->id_, infra);

    auto const [longest_prefix_routes, prefix_length] =
        get_longest_prefix_interlocking_routes(
            candidates, irs, train.sequence_points_,
            current_cover.stop_sequence_offset_);

    if (longest_prefix_routes.empty()) {
      return utls::unexpected(error::route_transform::NO_IR);
    }

    auto const prefix_ir_id = get_shortest_route(longest_prefix_routes, infra);
    current_ir = &irs.routes_[prefix_ir_id];

    result.emplace_back(current_ir->id_);

    // the current cover + the current interlocking route cover more
    // than the given stop sequence.
    // thus, it is fully covered
    auto const covered = current_cover.stop_sequence_offset_ +
                         current_ir->station_routes_.size();
    if (covered > train.sequence_points_.size()) break;

    // true when the current interlocking route and the currently worked on
    // station route end on the same node
    auto const last_sr_id =
        train.sequence_points_[current_cover.stop_sequence_offset_]
            .station_route_;
    auto const ends_align = current_ir->last_node(infra) ==
                            infra->station_routes_[last_sr_id]->nodes().back();

    // when the ends align we have completely inserted the current station route
    // into the interlocking path, so we can move on to the next station route
    current_cover.stop_sequence_offset_ +=
        ends_align ? prefix_length : prefix_length - 1;
    current_cover.node_idx_ = current_ir->end_offset_;
  }

  return result;
}

}  // namespace soro::tt