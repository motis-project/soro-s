#include "doctest/doctest.h"

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/filter.hpp"
#include "range/v3/view/transform.hpp"

#include "soro/utls/coroutine/coro_map.h"

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
  using namespace ranges;

  auto const sequence_point_nodes =
      train.sequence_points_ | views::transform([&](auto&& seq_point) {
        return seq_point.get_node(train.freight(), infra);
      }) |
      views::filter([](auto&& node_opt) { return node_opt.has_value(); }) |
      views::transform([](auto&& node_opt) { return node_opt.value(); }) |
      to<soro::vector<node::ptr>>();

  // all sequence points in a train must refer to a node
  CHECK_EQ(sequence_point_nodes.size(), train.sequence_points_.size());

  std::size_t spn_idx = 0;
  for (auto&& tn : train.iterate(infra)) {
    if (spn_idx < sequence_point_nodes.size() &&
        tn.node_ == sequence_point_nodes[spn_idx]) {
      CHECK(tn.sequence_point_.has_value());
      CHECK_EQ(*(*tn.sequence_point_)->get_node(train.freight(), infra),
               sequence_point_nodes[spn_idx]);
      ++spn_idx;
    }
  }

  // Check if all sequence points are contained in the interlocking routes
  CHECK_EQ(spn_idx, sequence_point_nodes.size());
}

void check_train_sequence_points(train const& train,
                                 infrastructure const& infra) {
  CHECK_MESSAGE(!train.sequence_points_.empty(),
                "Train sequence points can't be empty.");

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
    }

    if (sp.is_halt(sequence_point::type::PASSENGER)) {
      CHECK_MESSAGE(
          valid(sp.min_stop_time_),
          "Passenger halt sequence point needs a valid minimum stop time");
    }

    if (sp.is_halt(sequence_point::type::OPERATIONS)) {
      CHECK_MESSAGE(
          (sp.min_stop_time_ == duration2::zero()),
          "Operations halt sequence point have only zero minimum stop time");
    }

    auto const sp_node = sp.get_node(train.freight(), infra);
    CHECK_MESSAGE(sp_node.has_value(),
                  "Every sequence point needs a valid node");
    auto const sp_node_type = sp_node.value()->type();
    CHECK_MESSAGE(
        (is_runtime_checkpoint(sp_node_type) || is_halt(sp_node_type)),
        "Sequence points are only allowed for halts and runtime checkpoints.");
  }
}

void check_train(train const& train, infrastructure const& infra) {
  check_no_invalids(train);
  //  check_train_path_length(train, infra);
  check_train_sequence_points(train, infra);
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
  for (auto const& scenario : soro::test::get_timetable_scenarios()) {

#if defined(SERIALIZE)
    scenario->infra_->save("infra.raw");
    scenario->timetable_.save("tt.raw");

    infrastructure const infra("infra.raw");
    timetable const tt("tt.raw");
#else
    auto const& infra = *scenario->infra_;
    auto const& tt = scenario->timetable_;
#endif

    check_timetable(tt, infra);
  }
}

}  // namespace soro::tt::test
