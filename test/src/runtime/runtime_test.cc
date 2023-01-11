#include "doctest/doctest.h"

#include "utl/logging.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/runtime/runtime.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace utl;
using namespace soro;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;
using namespace soro::runtime;

void check_halt_count(train const& train, timestamps const& ts) {
  auto const halt_count = utls::count_if(
      ts.times_, [](auto&& stamp) { return stamp.element_->is(type::HALT); });

  CHECK_MESSAGE((train.total_halts() == halt_count),
                "There should be as many halt events in the timestamps as "
                "there are halts in the train run.");
}

void check_ascending_timestamps(timestamps const& ts) {
  if (ts.times_.empty()) {
    uLOG(utl::warn) << "Checking ascending timestamps with empty timestamps.";
    return;
  }

  auto last_time_stamp = ts.times_.front();
  for (auto idx = 1U; idx < ts.times_.size(); ++idx) {
    auto const& time_stamp = ts.times_[idx];

    CHECK_MESSAGE((time_stamp.arrival_ <= time_stamp.departure_),
                  "Arrival must happen before/same time as departure!");

    CHECK_MESSAGE(valid(time_stamp.arrival_), "No valid arrival timestamp.");
    CHECK_MESSAGE(valid(time_stamp.departure_), "No valid arrival timestamp.");

    if (last_time_stamp.element_->is_track_element() &&
        time_stamp.element_->is_track_element()) {
      auto const last_km = last_time_stamp.element_->get_km(nullptr);
      auto const km = time_stamp.element_->get_km(nullptr);

      if (last_km != km) {
        CHECK_MESSAGE((last_time_stamp.departure_ <= time_stamp.arrival_),
                      "Non increasing time stamps!");
      }
    } else {
      // TODO(julian) reenable this
      //      CHECK_MESSAGE((last_time_stamp.departure_ <= time_stamp.arrival_),
      //                    "Non increasing time stamps!");
    }

    last_time_stamp = time_stamp;
  }
}

void check_delays(infrastructure const& infra, timetable const& tt) {
  uLOG(utl::info) << "Checking delays";
  soro::size_t too_early_count = 0;
  soro::size_t delayed_count = 0;
  soro::size_t total_count = 0;

  duration2 max_delay = duration2::zero();
  duration2 max_too_early = duration2::zero();

  auto avg_too_early = duration2::zero();
  auto avg_delay = duration2::zero();

  for (auto const& train : tt->trains_) {
    auto const timestamps = runtime_calculation(train, infra, {type::HALT});

    if (timestamps.times_.empty()) {
      continue;
    }

    soro::size_t halt_id = 0;
    for (auto const& sp : train.sequence_points_) {
      if (!sp.is_halt()) {
        continue;
      }

      auto const& ts = timestamps.times_[timestamps.halt_indices_[halt_id]];

      ++total_count;
      if (valid(ts.departure_) && ts.departure_ > sp.departure_) {
        ++delayed_count;
        auto const delay = ts.departure_ - sp.departure_;
        max_delay = std::max(max_delay, delay);
        avg_delay += ts.departure_ - sp.departure_;
      }

      if (valid(ts.arrival_) && ts.arrival_ < sp.arrival_) {
        ++too_early_count;
        auto const too_early = sp.arrival_ - ts.arrival_;
        max_too_early = std::max(max_too_early, too_early);
        avg_too_early += sp.arrival_ - ts.arrival_;
      }

      ++halt_id;
    }
  }

  uLOG(info) << "Total halt timestamps: " << total_count;

  uLOG(info) << "Total delayed timestamps: " << delayed_count;
  uLOG(info) << "Total over punctual timestamps: " << too_early_count;

  if (avg_delay != duration2::zero()) {
    uLOG(info) << "AVG delay: " << avg_delay.count() / delayed_count;
  }

  if (avg_too_early != duration2::zero()) {
    uLOG(info) << "AVG too early: " << avg_too_early.count() / too_early_count;
  }

  uLOG(info) << "Maximum delay: " << max_delay.count();
  uLOG(info) << "Maximum over punctuality: " << max_too_early.count();
}

void test_event_existance_in_timestamps(train const& tr, timestamps const& ts,
                                        infrastructure const& infra) {
  std::set<type> event_types;

  for (auto const& timestamp : ts.times_) {
    event_types.insert(timestamp.element_->type());
  }

  soro::size_t ts_idx = 0;
  for (auto const& rich_node : tr.iterate(infra)) {
    if (rich_node.omitted()) {
      continue;
    }

    if (!event_types.contains(rich_node.node_->type())) {
      continue;
    }

    bool const same_element =
        ts.times_[ts_idx].element_->id() == rich_node.node_->element_->id();

    CHECK_MESSAGE(
        same_element,
        "Element from timestamps does not correspond to train run element");

    ++ts_idx;
  }

  CHECK_MESSAGE((ts_idx == ts.times_.size()), "Did not check every timestamp");
}

void check_runtime_with_events(infrastructure const& infra, timetable const& tt,
                               type_set const& record_events) {
  for (auto const& train : tt->trains_) {
    auto const timestamps = runtime_calculation(train, infra, record_events);

    if (record_events.contains(type::HALT)) {
      check_halt_count(train, timestamps);
    }

    check_ascending_timestamps(timestamps);

    test_event_existance_in_timestamps(train, timestamps, infra);
  }
}

void check_runtime(infrastructure const& infra, timetable const& tt) {
  check_runtime_with_events(infra, tt, {type::HALT});
  check_runtime_with_events(
      infra, tt,
      {type::RUNTIME_CHECKPOINT_UNDIRECTED, type::RUNTIME_CHECKPOINT,
       type::APPROACH_SIGNAL, type::MAIN_SIGNAL, type::EOTD});
  check_runtime_with_events(infra, tt, type_set{all_types()});
}

TEST_CASE("runtime") {
  for (auto const& scenario : soro::test::get_timetable_scenarios()) {
    check_runtime(*scenario->infra_, scenario->timetable_);
    check_delays(*scenario->infra_, scenario->timetable_);
  }
}
