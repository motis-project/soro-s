#include "doctest/doctest.h"

#include "soro/utls/coroutine/coro_map.h"

#include "utl/pipes.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/length.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"
#include "test/utls/utls.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;

namespace soro::tt::test {

void check_arrival_departures(train const& train) {
  CHECK_MESSAGE((train.stop_times_.front().arrival_ == INVALID_TIME),
                "First arrival must be invalid");
  CHECK_MESSAGE((train.stop_times_.front().departure_ != INVALID_TIME),
                "First departure must be valid");
  CHECK_MESSAGE((train.stop_times_.back().arrival_ != INVALID_TIME),
                "Last arrival must be valid");
  CHECK_MESSAGE((train.stop_times_.back().departure_ == INVALID_TIME),
                "Last departure must be invalid");

  std::size_t idx = 0;
  for (auto const [arr, dep, mst] : train.stop_times_) {
    if (idx == 0 || idx == train.stop_times_.size() - 1) {
      continue;
    }

    ++idx;
    bool const both_invalid = arr == INVALID_TIME && dep == INVALID_TIME;
    bool const both_valid = arr != INVALID_TIME && dep != INVALID_TIME;
    bool const either_or = both_valid || both_invalid;
    CHECK_MESSAGE(either_or, "Arr/Dep must both be valid or invalid");
  }
}

void check_no_invalids(train const& train) {
  for (auto const& entry : train.path_.entries_) {
    CHECK_MESSAGE(interlocking_route::valid(entry.interlocking_id_),
                  "Signal station route is nullptr!");

    for (auto const& sp : entry.sequence_points_) {
      CHECK(station_route::valid(sp.station_route_));
    }
  }
}

void check_train_path_length(train const& train, infrastructure const& infra) {
  auto const e1 = get_path_length_from_elements(
      utls::coro_map(train.iterate(infra), [](auto&& rn) { return rn.node_; }));

  CHECK_MESSAGE((train.path_.length_ == e1),
                "Different lengths from the two length calculation funs");
}

void check_train_path_sequence_points(train const& train,
                                      infrastructure const& infra) {
  for (auto const& entry : train.path_.entries_) {
    auto const& ir =
        infra->interlocking_.interlocking_routes_[entry.interlocking_id_];

    auto const sequence_point_nodes =
        utl::all(entry.sequence_points_) |
        utl::transform([&](auto&& seq_point) {
          return seq_point.get_node(train.freight(), infra);
        }) |
        utl::remove_if([](auto&& node_opt) { return !node_opt.has_value(); }) |
        utl::transform([](auto&& node_opt) { return node_opt.value(); }) |
        utl::vec();

    std::size_t spn_idx = 0;
    for (auto const rn : ir.iterate(infra)) {
      if (rn.node_ == sequence_point_nodes[spn_idx]) {
        ++spn_idx;
      }
    }

    // Check if all sequence points are contained in the interlocking route
    CHECK_EQ(spn_idx, sequence_point_nodes.size());
  }
}

void check_train(train const& train, infrastructure const& infra) {
  //  check_no_invalids(train);
  //  check_arrival_departures(train);
  //  check_train_path_length(train, infra);
  check_train_path_sequence_points(train, infra);
}

void check_timetable(timetable const& tt, infrastructure const& infra) {
  for (auto const& train : tt->trains_) {
    check_train(*train, infra);
  }

  //  soro::test::utls::check_continuous_ascending_ids(tt->trains_);
}

TEST_CASE("timetable test") {
  for (auto const& scenario :
       soro::test::get_timetable_scenarios(soro::test::DE_SCENARIO)) {
    check_timetable(scenario.timetable_, scenario.infra_);
  }
}

}  // namespace soro::tt::test
