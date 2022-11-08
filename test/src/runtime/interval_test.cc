#include "doctest/doctest.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/runtime/interval.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::si;
using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;

void check_interval_list(infrastructure const& infra, timetable const& tt) {
  for (auto const& train : tt->trains_) {
    type_set const event_types({type::HALT, type::EOTD, type::MAIN_SIGNAL});
    type_set const border_types(
        {type::HALT, type::MAIN_SIGNAL, type::APPROACH_SIGNAL});

    auto const& intervals =
        get_interval_list(*train, event_types, border_types, infra);

    CHECK_MESSAGE(
        intervals.back().distance_ == train->path_length(),
        "Last interval length should be the same length as total train"
        "path length");

    length last_distance = intervals.front().distance_;
    for (auto const& interval : intervals) {
      CHECK_MESSAGE(interval.speed_limit_ != ZERO<speed>,
                    "Found interval with speed limit 0!");
      CHECK_MESSAGE(valid(interval.speed_limit_),
                    "Found interval with invalid speed limit!");

      CHECK_MESSAGE(last_distance <= interval.distance_,
                    "Found non increasing distance");

      for (auto const& e : interval.events_) {
        CHECK_MESSAGE(e.distance_ <= interval.distance_,
                      "Event distance is larger than interval distance!");
        CHECK_MESSAGE(e.distance_ >= last_distance,
                      "Event distance smaller than last distance!");
        last_distance = e.distance_;
      }

      last_distance = interval.distance_;
    }

    std::map<type, size_t> total_path_count;
    std::map<type, size_t> intervals_count;

    for (auto const& type : event_types) {
      total_path_count.insert(std::pair(type, 0));
      intervals_count.insert(std::pair(type, 0));
    }

    for (auto const& rn : train->iterate(skip_omitted::ON)) {
      auto const it = total_path_count.find(rn.node_->element_->type());
      if (it != std::cend(total_path_count)) {
        total_path_count[rn.node_->element_->type()] += 1;
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
      CHECK_MESSAGE(intervals_count[type] == total_path_count[type],
                    "Event type count in intervals must be equal "
                    "than total path events!");
    }
  }
}

TEST_SUITE("interval") {
  TEST_CASE("interval case") {
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(FOLLOW_OPTS, infra);
    check_interval_list(infra, tt);
  }
}