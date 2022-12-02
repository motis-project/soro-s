#include "doctest/doctest.h"

#include "soro/utls/coroutine/coro_map.h"

#include "utl/pipes.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/length.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"
#include "test/timetable/train_iterator_test.h"
#include "test/utls/utls.h"

using namespace soro::utls;
using namespace soro::infra;

namespace soro::tt::test {

void check_no_invalids(train const& train) {
  for (auto const& ir_id : train.path_) {
    CHECK_MESSAGE(interlocking_route::valid(ir_id),
                  "Interlocking route is nullptr!");
  }

  for (auto const& sp : train.sequence_points_) {
    CHECK(station_route::valid(sp.station_route_));
  }
}

void check_train_path_length(train const& train, infrastructure const& infra) {
  auto const e1 = get_path_length_from_elements(
      utls::coro_map(train.iterate(infra), [](auto&& rn) { return rn.node_; }));

  CHECK_MESSAGE((train.length_ == e1),
                "Different lengths from the two length calculation funs");
}

void check_train_path_sequence_points(train const& train,
                                      infrastructure const& infra) {
  auto const sequence_point_nodes =
      utl::all(train.sequence_points_) | utl::transform([&](auto&& seq_point) {
        return seq_point.get_node(train.freight(), infra);
      }) |
      utl::remove_if([](auto&& node_opt) { return !node_opt.has_value(); }) |
      utl::transform([](auto&& node_opt) { return node_opt.value(); }) |
      utl::vec();

  // all sequence points in a train must refer to a node
  CHECK_EQ(sequence_point_nodes.size(), train.sequence_points_.size());

  std::size_t spn_idx = 0;
  for (auto const ir_id : train.path_) {
    auto const& ir = infra->interlocking_.routes_[ir_id];

    for (auto const rn : ir.iterate(infra)) {
      if (rn.node_ == sequence_point_nodes[spn_idx]) {
        ++spn_idx;
      }
    }
  }

  // Check if all sequence points are contained in the interlocking routes
  CHECK_EQ(spn_idx, sequence_point_nodes.size());
}

void check_train_sequence_points(train const& train) {
  if (!train.break_in_) {
    CHECK_MESSAGE(
        train.sequence_points_.front().is_halt(),
        "Train that is not breaking in needs a stop at first sequence point");
  }

  if (!train.break_out_) {
    CHECK_MESSAGE(
        train.sequence_points_.back().is_halt(),
        "Train that is not breaking out needs a stop at last sequence point");
  }

  for (auto const& sp : train.sequence_points_) {
    CHECK_MESSAGE(valid(sp.departure_),
                  "Every sequence point needs a valid departure");

    if (sp.is_halt()) {
      CHECK_MESSAGE(valid(sp.arrival_),
                    "Every halt sequence point needs a valid arrival");
      CHECK_MESSAGE(
          valid(sp.min_stop_time_),
          "Every halt sequence point needs a valid minimum stop time");
    }
  }
}

void check_train(train const& train, infrastructure const& infra) {
  check_no_invalids(train);
  //  check_train_path_length(train, infra);
  check_train_sequence_points(train);
  check_train_path_sequence_points(train, infra);

  do_train_iterator_tests(train, infra);
}

void check_timetable(timetable const& tt, infrastructure const& infra) {
  for (auto const& train : tt->trains_) {
    check_train(train, infra);
  }

  soro::test::utls::check_continuous_ascending_ids(tt->trains_);
}

TEST_CASE("timetable test") {
  for (auto const& scenario :
       soro::test::get_timetable_scenarios(soro::test::DE_SCENARIO)) {
    std::cout << "scnearion\n";
    check_timetable(scenario.timetable_, scenario.infra_);
  }
  std::cout << "Hi\n";
}

}  // namespace soro::tt::test
