#include "doctest/doctest.h"

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/transform.hpp"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/timetable/sequence_point.h"
#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

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
    CHECK_NE(sp.station_route_, station_route::invalid());
  }
}

void check_train_path_sequence_points(train const& train,
                                      infrastructure const& infra) {
  using namespace ranges;

  auto const sequence_point_nodes = train.sequence_points_ |
                                    views::transform([&](auto&& seq_point) {
                                      return seq_point.get_node(infra);
                                    }) |
                                    to<soro::vector<node::ptr>>();

  // all sequence points in a train must refer to a node
  CHECK_EQ(sequence_point_nodes.size(), train.sequence_points_.size());

  soro::size_t spn_idx = 0;
  for (auto&& tn : train.iterate(infra)) {
    if (spn_idx < sequence_point_nodes.size() &&
        tn.node_ == sequence_point_nodes[spn_idx]) {
      CHECK(tn.sequence_point_.has_value());
      CHECK_EQ((*tn.sequence_point_)->get_node(infra),
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
        "Train that is not breaking in needs a halt at first sequence point");
  }

  if (!train.break_out_) {
    CHECK_MESSAGE(
        train.sequence_points_.back().is_halt(),
        "Train that is not breaking out needs a halt at last sequence point");
  }

  for (auto const& sp : train.sequence_points_) {
    CHECK_MESSAGE((sp.idx_ != station_route::invalid_idx()),
                  "every point needs a valid node");
    CHECK_MESSAGE((sp.station_route_ != station_route::invalid()),
                  "every point needs a valid station route");
    auto const sr = infra->station_routes_[sp.station_route_];
    CHECK_MESSAGE((sp.idx_ < sr->size()),
                  "every point needs a valid node index");

    CHECK_MESSAGE(valid(sp.min_stop_time_),
                  "every point needs a valid min stop time");

    CHECK_MESSAGE(((sp.arrival_.has_value() && sp.departure_.has_value()) ||
                   (!sp.arrival_.has_value() && !sp.departure_.has_value())),
                  "either arrival and departure are set or none");

    if (sp.is_halt_type(sequence_point::type::PASSENGER)) {
      CHECK_MESSAGE(sp.arrival_.has_value(),
                    "passenger halt point needs a valid arrival");
      CHECK_MESSAGE(sp.departure_.has_value(),
                    "passenger halt point needs a valid departure");
    }

    if (sp.is_halt_type(sequence_point::type::OPERATIONS)) {
      CHECK_MESSAGE((sp.min_stop_time_ == duration::zero()),
                    "operations halt point must have zero min stop time");
    }

    if (sp.is_halt_type(sequence_point::type::ADDITIONAL)) {
      CHECK_MESSAGE(!sp.arrival_.has_value(),
                    "additional halt points don't have a arrival time");
      CHECK_MESSAGE(!sp.departure_.has_value(),
                    "additional halt points don't have a departure time");
    }

    if (sp.is_halt_type(sequence_point::type::TRANSIT)) {
      CHECK_MESSAGE(sp.arrival_.has_value(),
                    "transit points need a arrival time");
      CHECK_MESSAGE(sp.departure_.has_value(),
                    "transit points need a departure time");
      CHECK_MESSAGE((sp.min_stop_time_ == duration::zero()),
                    "transit points must have a min stop time of 0");
      CHECK_MESSAGE((sp.departure_.value() == sp.arrival_.value()),
                    "transit points require arrival == departure");
    }
  }
}

void check_train(train const& train, infrastructure const& infra) {
  check_no_invalids(train);
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
