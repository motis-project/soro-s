#include "test/ordering/check_train_edge_transitivity.h"

#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/filter.hpp"
#include "range/v3/view/transform.hpp"

#include "soro/base/soro_types.h"

#include "soro/utls/algo/sort_and_intersect.h"
#include "soro/utls/graph/traversal.h"
#include "soro/utls/print_progress.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/find_if.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/station/station.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/ordering/graph.h"

namespace soro::ordering::test {

using namespace soro::tt;
using namespace soro::infra;

soro::optional<station::id> get_meeting_station(
    std::vector<interlocking_route::id> const& ids,
    infrastructure const& infra) {
  auto const get_ir_stations = [&](auto&& ir_id) {
    auto const& ir = infra->interlocking_.routes_[ir_id];
    soro::vector<station::id> result;
    for (auto const sr_id : ir.station_routes_) {
      auto const& sr = infra->station_routes_[sr_id];
      result.emplace_back(sr->station_->id_);
    }
    return result;
  };

  soro::optional<station::id> result;

  if (ids.empty()) return result;

  auto stations = get_ir_stations(ids.front());

  for (auto const& id : ids) {
    stations = utls::sort_and_intersect(stations, get_ir_stations(id));
  }

  if (stations.size() == 1) result.emplace(stations.front());

  return result;
}

bool is_3_node_sync(graph::node const& node, graph const& og,
                    infrastructure const& infra, timetable const& tt) {

  /*
   * checks if we have the following situation,
   * where the train edge between [A] -> [B] has been removed
   *
   * Train 1  [A] -> [B]
   *          |       ^
   *          + ----+ |
   *                | |
   *                v |
   * Train 2 [C] -> [D]
   * but it is correct since train 1 and train 2 are syncing up
   */

  auto const& A = node;  // NOLINT

  if (!A.has_next(og)) return false;

  auto const& B = A.next(og);  // NOLINT

  auto const D_it = utls::find_if(A.out(og), [&](auto&& o) {  // NOLINT
    auto const& D = og.nodes_[o];  // NOLINT

    auto const diff_train = A.get_train_id(og) != D.get_train_id(og);
    auto const D_to_B = utls::contains(D.out(og), B.get_id(og));

    return diff_train && D_to_B;
  });

  if (D_it == std::end(A.out(og))) return false;

  auto const& D = og.nodes_[*D_it];  // NOLINT

  auto const meetup_opt =
      get_meeting_station({A.ir_id_, B.ir_id_, D.ir_id_}, infra);

  if (!meetup_opt.has_value()) return false;

  auto const meetup = *meetup_opt;

  auto const& train1 = tt->trains_[A.get_trip_group(og).train_id_];
  auto const& train2 = tt->trains_[D.get_trip_group(og).train_id_];

  if (!train1.goes_through(meetup, infra) ||
      !train2.goes_through(meetup, infra)) {
    return false;
  }

  auto const t1_sps = train1.get_sequence_points(meetup, infra);
  auto const t2_sps = train2.get_sequence_points(meetup, infra);

  auto A_sp = utls::find_if(  // NOLINT
      t1_sps, [](auto&& p) { return p.is_measurable(); });
  auto D_sp = utls::find_if(  // NOLINT
      t2_sps, [](auto&& p) { return p.is_measurable(); });

  if (A_sp == std::end(t1_sps) || D_sp == std::end(t2_sps)) return false;

  auto const is_3_node_sync = *A_sp->arrival_ < *D_sp->departure_ &&
                              *D_sp->arrival_ < *A_sp->departure_;

  return is_3_node_sync;
}

bool is_4_node_overtake(graph::node const& node, graph const& og,
                        infrastructure const& infra, timetable const& tt) {
  /*
   * checks if we have the following situation,
   * where the train edge between [A] -> [B] has been removed
   * Train 1 [A] -> [B]
   *          |      ^
   *          v      |
   * Train 2 [C] -> [D]
   * but it is correct since train 2 overtakes train 1
   */

  auto const& A = node;  // NOLINT

  if (!A.has_next(og)) return false;

  auto const& B = A.next(og);  // NOLINT

  auto const C_it = utls::find_if(A.out(og), [&](auto&& o) {  // NOLINT
    auto const& C = og.nodes_[o];  // NOLINT

    auto const diff_train = A.get_train_id(og) != C.get_train_id(og);
    auto const D_to_B =
        C.has_next(og) && utls::contains(C.next(og).out(og), B.get_id(og));

    return diff_train && D_to_B;
  });

  if (C_it == std::end(A.out(og))) return false;

  auto const& C = og.nodes_[*C_it];  // NOLINT

  if (!C.has_next(og)) return false;

  auto const& D = C.next(og);  // NOLINT

  auto const& train1 = tt->trains_[A.get_trip_group(og).train_id_];
  auto const& train2 = tt->trains_[C.get_trip_group(og).train_id_];

  auto const meetup_opt =
      get_meeting_station({A.ir_id_, B.ir_id_, C.ir_id_, D.ir_id_}, infra);

  if (!meetup_opt.has_value()) return false;

  auto const meetup = *meetup_opt;

  if (!train1.goes_through(meetup, infra) ||
      !train2.goes_through(meetup, infra)) {
    return false;
  }

  auto const t1_sps = train1.get_sequence_points(meetup, infra);
  auto const t2_sps = train2.get_sequence_points(meetup, infra);

  auto A_sp = utls::find_if(  // NOLINT
      t1_sps, [](auto&& p) { return p.is_measurable(); });
  auto C_sp = utls::find_if(  // NOLINT
      t2_sps, [](auto&& p) { return p.is_measurable(); });

  if (A_sp == std::end(t1_sps) || C_sp == std::end(t2_sps)) return false;

  auto const is_overtake = *A_sp->arrival_ < *C_sp->arrival_ &&
                           *C_sp->arrival_ <= *C_sp->departure_ &&
                           *C_sp->departure_ < *A_sp->departure_;

  return is_overtake;
}

std::vector<graph::edge> get_transitive_edges(graph::node::id const start,
                                              graph::edges_t const& outgoing) {
  using namespace ranges;

  soro::vector_map<graph::node::id, uint32_t> max_depth(outgoing.size(), 0);

  max_depth[start] = 0;

  auto const work_on_node = [&max_depth](auto&& to, auto&& from) {
    max_depth[to] = std::max(max_depth[to], max_depth[from] + 1);
    return false;
  };

  auto const work_on_edge = [&max_depth](auto&& from, auto&& to) {
    max_depth[to] = std::max(max_depth[to], max_depth[from] + 1);
    return false;
  };

  auto const get_neighbours = [&outgoing](auto&& node_id) {
    return outgoing[node_id];
  };

  utls::dfs(start, get_neighbours, work_on_node, work_on_edge);

  auto const bucket = outgoing[start];
  return bucket | views::filter([&](auto&& to) { return max_depth[to] > 1; }) |
         views::transform([&](auto&& to) -> graph::edge {
           return {.from_ = start, .to_ = to};
         }) |
         ranges::to<std::vector>();
}

void print_faulty_train_numbers(std::set<train::id> const& faulty_train_ids,
                                timetable const& tt,
                                std::string const& reason) {

  std::cout << "Faulty Train Numbers:\n";
  for (auto const& id : faulty_train_ids) {
    auto const& number = tt->trains_[id].number_;
    std::cout << number.main_ << "," << number.sub_ << "," << reason << "\n";
  }
}

void print_faulty_train_id(ordering::graph::node const& node,
                           ordering::graph const& og) {

  std::cout << "train edge missing for train : "
            << node.get_trip_group(og).train_id_
            << ", but got out edges to train [";

  for (auto const& out : node.out(og)) {
    std::cout << og.nodes_[out].get_trip_group(og).train_id_ << " ";
  }
  std::cout << "]\n";

  std::cout << "train edge missing for train : "
            << node.get_trip_group(og).train_id_
            << ", but got inc edges from train [";

  for (auto const& in : node.in(og)) {
    std::cout << og.nodes_[in].get_trip_group(og).train_id_ << " ";
  }

  std::cout << "]\n";
}

bool check_train_edge_transitivity(graph const& og, infrastructure const& infra,
                                   timetable const& tt) {
  std::set<train::id> faulty_train_ids;

  for (auto const& node : og.nodes_) {
    utls::print_progress("checking transitive train edges", og.nodes_);

    // no train edge here, continue
    if (!node.has_next(og)) continue;

    auto const transitive_edges =
        get_transitive_edges(node.get_id(og), og.outgoing_edges_);

    auto const train_edge_is_transitive = utls::contains(
        transitive_edges, graph::edge{node.get_id(og), node.next_id(og)});

    if (!train_edge_is_transitive) continue;

    auto const is_overtake = is_4_node_overtake(node, og, infra, tt);
    auto const is_sync = is_3_node_sync(node, og, infra, tt);

    if (!is_overtake && !is_sync) {
      faulty_train_ids.insert(node.get_trip_group(og).train_id_);
      print_faulty_train_id(node, og);
    }
  }

  if (!faulty_train_ids.empty()) {
    print_faulty_train_numbers(faulty_train_ids, tt, "transitive_train_edge");
  }

  return faulty_train_ids.empty();
}

}  // namespace soro::ordering::test