#include "doctest/doctest.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/runtime/interval.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro::si;
using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;

void check_interval_list(infrastructure const& infra, timetable const& tt) {
  for (auto const& train : tt->trains_) {
    type_set const event_types({type::HALT, type::EOTD, type::MAIN_SIGNAL});

    auto const& intervals = get_interval_list(train, event_types, infra);

    CHECK_MESSAGE(
        (intervals.back().distance_ == train.path_length(infra)),
        "Last interval length should be the same length as total train"
        "path length");

    length last_distance = intervals.front().distance_;
    for (auto idx = 1U; idx < intervals.size(); ++idx) {
      auto const& interval = intervals[idx];

      CHECK_MESSAGE(valid(interval.limit_right_),
                    "Found interval with speed limit 0!");
      CHECK_MESSAGE(valid(interval.limit_left_),
                    "Found interval with speed limit 0!");
      CHECK_MESSAGE((interval.limit_right_ != ZERO<speed>),
                    "Found interval with speed limit 0!");
      CHECK_MESSAGE((interval.limit_left_ != ZERO<speed>),
                    "Found interval with speed limit 0!");

      CHECK_MESSAGE(valid(interval.distance_), "Found non valid distance");
      CHECK_MESSAGE((last_distance <= interval.distance_),
                    "Found non increasing distance");
      CHECK_MESSAGE(!is_zero(interval.distance_ - last_distance),
                    "No 0 length intervals");

      if (interval.is_halt()) {
        CHECK_MESSAGE(interval.sequence_point_.has_value(),
                      "Halt interval needs sequence point.");
        CHECK_MESSAGE((*interval.sequence_point_)->is_halt(),
                      "Halt interval needs halt sequence point.");
      }

      for (auto const& e : interval.events_) {
        CHECK_MESSAGE((e.distance_ <= interval.distance_),
                      "Event distance is larger than interval distance!");
        CHECK_MESSAGE((e.distance_ >= last_distance),
                      "Event distance smaller than last distance!");
        last_distance = e.distance_;
      }

      last_distance = interval.distance_;
    }

    std::map<type, soro::size_t> total_path_count;
    std::map<type, soro::size_t> intervals_count;

    for (auto const& type : event_types) {
      total_path_count.insert(std::pair(type, 0));
      intervals_count.insert(std::pair(type, 0));
    }

    for (auto const& tn : train.iterate(infra)) {
      if (tn.omitted()) {
        continue;
      }

      auto const it = total_path_count.find(tn.node_->element_->type());
      if (it != std::cend(total_path_count)) {
        total_path_count[tn.node_->element_->type()] += 1;
      }
    }

    for (auto const& interval : intervals) {
      for (auto const& event : interval.events_) {
        auto const it = intervals_count.find(event.node_->element_->type());
        if (it != std::cend(intervals_count)) {
          intervals_count[event.node_->element_->type()] += 1;
        }
      }
    }

    for (auto const& type : event_types) {
      CHECK_MESSAGE((intervals_count[type] == total_path_count[type]),
                    "Event type count in intervals must be equal "
                    "than total path events!");
    }
  }
}

TEST_CASE("interval") {
  for (auto const& scenario : soro::test::get_timetable_scenarios()) {
    check_interval_list(*scenario->infra_, scenario->timetable_);
  }
}