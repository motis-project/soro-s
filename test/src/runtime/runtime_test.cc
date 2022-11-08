#include "doctest/doctest.h"

#include "utl/logging.h"

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
  auto const halt_count = std::count_if(
      std::cbegin(ts.times_), std::cend(ts.times_),
      [](auto&& stamp) { return stamp.element_->is(type::HALT); });

  CHECK_MESSAGE(train.total_halts() == halt_count,
                "There should be as many halt events in the timestamps as "
                "there are halts in the train run.");
}

void check_ascending_timestamps(timestamps const& ts) {
  auto last_time_stamp = ts.times_.front();
  for (size_t idx = 1; idx < ts.times_.size(); ++idx) {
    auto const& time_stamp = ts.times_[idx];

    CHECK_MESSAGE(time_stamp.arrival_ <= time_stamp.departure_,
                  "Arrival must happen before/same time as departure!");

    CHECK_MESSAGE(time_stamp.arrival_ > 0, "No negative timestamps allowed!");
    CHECK_MESSAGE(time_stamp.departure_ > 0, "No negative timestamps allowed!");

    if (valid(last_time_stamp.arrival_)) {
      CHECK_MESSAGE(last_time_stamp.arrival_ <= time_stamp.arrival_,
                    "Non increasing time stamps!");
    }

    if (valid(time_stamp.departure_)) {
      CHECK_MESSAGE(last_time_stamp.departure_ <= time_stamp.departure_,
                    "Non increasing time stamps!");
    }

    last_time_stamp = time_stamp;
  }
}

void check_delays(infrastructure const& infra, timetable const& tt) {
  size_t too_early_count = 0;
  size_t delayed_count = 0;
  size_t total_count = 0;

  duration max_delay = duration{0};
  duration max_too_early = duration{0};

  for (auto const& train : tt) {
    auto const timestamps = runtime_calculation(*train, infra, {type::HALT});

    size_t halt_id = 0;
    for (auto const& stop_time : train->stop_times_) {
      if (!stop_time.is_halt()) {
        continue;
      }

      auto const& ts = timestamps.times_[timestamps.halt_indices_[halt_id]];

      ++total_count;
      if (valid(ts.departure_) && ts.departure_ > stop_time.departure_) {
        ++delayed_count;
        duration const delay =
            (ts.departure_ - stop_time.departure_).as_duration();
        max_delay = std::max(max_delay, delay);
      }

      if (valid(ts.arrival_) && ts.arrival_ < stop_time.arrival_) {
        ++too_early_count;
        auto const too_early = (stop_time.arrival_ - ts.arrival_).as_duration();
        max_too_early = std::max(max_too_early, too_early);
      }

      ++halt_id;
    }
  }

  uLOG(info) << "Total halt timestamps: " << total_count;

  uLOG(info) << "Total delayed timestamps: " << delayed_count;
  uLOG(info) << "Total over punctual timestamps: " << too_early_count;

  uLOG(info) << "Maximum delay: " << max_delay;
  uLOG(info) << "Maximum over punctuality: " << max_too_early;
}

void test_event_existance_in_timestamps(train const& tr, timestamps const& ts) {
  std::set<type> event_types;

  for (auto const& timestamp : ts.times_) {
    event_types.insert(timestamp.element_->type());
  }

  size_t ts_idx = 0;
  for (auto const& rich_node : tr.iterate(skip_omitted::ON)) {
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

  CHECK_MESSAGE(ts_idx == ts.times_.size(), "Did not check every timestamp");
}

void check_runtime_with_events(infrastructure const& infra, timetable const& tt,
                               type_set const& record_events) {
  for (auto const& train : tt) {
    auto const timestamps = runtime_calculation(*train, infra, record_events);

    if (record_events.contains(type::HALT)) {
      check_halt_count(*train, timestamps);
    }

    check_ascending_timestamps(timestamps);

    test_event_existance_in_timestamps(*train, timestamps);
  }
}

void check_runtime(infrastructure const& infra, timetable const& tt) {
  check_runtime_with_events(infra, tt, {type::HALT});
  check_runtime_with_events(
      infra, tt, {type::APPROACH_SIGNAL, type::MAIN_SIGNAL, type::EOTD});
  check_runtime_with_events(infra, tt, type_set{all_types()});
}

TEST_SUITE("overtake runtime") {
  TEST_CASE("overtake") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(OVERTAKE_OPTS, infra);
    check_runtime(infra, tt);
    check_delays(infra, tt);
  }

  TEST_CASE("overtake archive") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(OVERTAKE_OPTS_ARCHIVE, infra);
    check_runtime(infra, tt);
    check_delays(infra, tt);
  }
}

TEST_SUITE("follow runtime") {
  TEST_CASE("follow") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(FOLLOW_OPTS, infra);
    check_runtime(infra, tt);
    check_delays(infra, tt);
  }

  TEST_CASE("follow archive") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(FOLLOW_OPTS_ARCHIVE, infra);
    check_runtime(infra, tt);
    check_delays(infra, tt);
  }
}
