#include "doctest/doctest.h"

#include <iostream>
#include <iterator>
#include <ranges>

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/serializable.h"
#include "soro/utls/std_wrapper/accumulate.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/interval.h"
#include "soro/timetable/sequence_point.h"
#include "soro/timetable/timetable.h"

#include "soro/ordering/graph.h"

#include "soro/simulation/graph/graph.h"

#include "test/file_paths.h"
#include "test/utls/get_cycle.h"
#include "test/utls/has_cycle.h"

namespace soro::sim::test {

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

soro::size_t get_total_train_dependencies(graph const& sg) {
  return utls::accumulate(
      sg.train_dependencies_, soro::size_t{0},
      [](auto&& acc, auto&& bucket) { return acc + bucket.size(); });
}

soro::size_t get_total_timetable_dependencies(graph const& sg) {
  return utls::accumulate(
      sg.timetable_dependencies_, soro::size_t{0},
      [](auto&& acc, auto&& train_dep) { return acc + train_dep.has_value(); });
}

void check_timetable_dependencies(graph const& sg, timetable const& tt) {
  for (auto const& trip_group : sg.trips_) {
    // gather all sequence points from the simulation graph
    std::vector<sequence_point> sps;
    for (auto const& node : trip_group.nodes(sg)) {
      if (sg.timetable_dependencies_[node.get_id(sg)].has_value()) {
        sps.emplace_back(*sg.timetable_dependencies_[node.get_id(sg)].value());
      }
    }

    auto range = tt->trains_[trip_group.train_id_].sequence_points_ |
                 std::views::filter([](auto&& sp) { return sp.is_stop(); });
    auto const tt_sps =
        std::vector<sequence_point>(std::begin(range), std::end(range));

    if (sps != tt_sps) {
      std::cout << "train id: " << trip_group.train_id_ << "\n";
    }

    CHECK_EQ(sps, tt_sps);
  }
}

void print_train_cycles(graph const& sg) {
  auto const cycles = ::soro::test::get_cycles(sg);
  std::cout << "found " << cycles.size() << " cycles\n";

  std::vector<std::set<tt::train::id>> cycle_trains;

  for (auto const& cycle : cycles) {
    std::set<tt::train::id> train_ids;

    for (auto const& n_id : cycle) {
      auto const& node = sg.nodes_[n_id];
      train_ids.insert(node.get_trip_group(sg).train_id_);
    }

    cycle_trains.push_back(train_ids);
  }

  std::cout << "cycle train ids:\n";
  for (auto const& train_ids : cycle_trains) {
    std::cout << '[';

    for (auto const& train_id : train_ids) {
      std::cout << train_id << ',';
    }

    std::cout << "]\n";
  }
}

void check_for_cycle(graph const& sg) {
  auto const found_cycle = ::soro::test::has_cycle(sg);

  if (found_cycle) print_train_cycles(sg);

  CHECK(!found_cycle);
}

void check_sim_graph(graph const& sg, ordering::graph const&,
                     timetable const& tt, infrastructure const&) {
  check_for_cycle(sg);
  check_timetable_dependencies(sg, tt);
}

TEST_SUITE("simulation graph") {
  TEST_CASE("simulation graph, follow") {
    auto opts = soro::test::SMALL_OPTS;
    auto tt_opts = soro::test::FOLLOW_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    ordering::graph const og(infra, tt);
    graph const sg(infra, tt, og);

    auto const total_train_dependencies = get_total_train_dependencies(sg);
    CHECK_EQ(total_train_dependencies, 5);

    auto const total_tt_dependencies = get_total_timetable_dependencies(sg);
    CHECK_EQ(total_tt_dependencies, 6);

    check_sim_graph(sg, og, tt, infra);
  }

  TEST_CASE("simulation graph, cross") {
    auto opts = soro::test::SMALL_OPTS;
    auto tt_opts = soro::test::CROSS_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    ordering::graph const og(infra, tt);
    graph const sg(infra, tt, og);

    auto const total_train_dependencies = get_total_train_dependencies(sg);
    CHECK_EQ(total_train_dependencies, 2);

    auto const total_tt_dependencies = get_total_timetable_dependencies(sg);
    CHECK_EQ(total_tt_dependencies, 6);

    check_sim_graph(sg, og, tt, infra);
  }

  TEST_CASE("simulation de_kss 2h") {
    using namespace date;

    interval const inter{
        .start_ = ymd_to_abs(2021_y / November / 13) + hours{7},
        .end_ = ymd_to_abs(2021_y / November / 13) + hours{9}};

    ordering::graph::filter const filter{
        .interval_ = inter, .include_trains_ = {}, .exclude_trains_ = {}};

    auto const infra = utls::try_deserializing<infrastructure>(
        "infra.raw", soro::test::DE_ISS_OPTS);

    auto const tt = utls::try_deserializing<timetable>(
        "timetable.raw", soro::test::DE_KSS_OPTS, infra);

    ordering::graph const og(infra, tt, filter);
    sim::graph const sg(infra, tt, og);

    check_sim_graph(sg, og, tt, infra);
  }

  TEST_CASE("simulation de_kss 24h") {
    using namespace date;

    interval const inter{
        .start_ = ymd_to_abs(2021_y / November / 13),
        .end_ = ymd_to_abs(2021_y / November / 13) + hours{24}};

    ordering::graph::filter const filter{
        .interval_ = inter, .include_trains_ = {}, .exclude_trains_ = {}};

    auto const infra = utls::try_deserializing<infrastructure>(
        "infra.raw", soro::test::DE_ISS_OPTS);

    auto const tt = utls::try_deserializing<timetable>(
        "timetable.raw", soro::test::DE_KSS_OPTS, infra);

    ordering::graph const og(infra, tt, filter);
    sim::graph const sg(infra, tt, og);

    check_sim_graph(sg, og, tt, infra);
  }
}

}  // namespace soro::sim::test