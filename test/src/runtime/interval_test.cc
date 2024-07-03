#include "doctest/doctest.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <utility>

#include "utl/pairwise.h"

#include "soro/base/soro_types.h"

#include "soro/utls/std_wrapper/find_if.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/runtime/common/get_intervals.h"
#include "soro/runtime/common/interval.h"

#include "test/file_paths.h"

namespace soro::runtime::test {

using namespace soro;
using namespace soro::si;
using namespace soro::tt;
using namespace soro::infra;
using namespace soro::test;

void check_signal_speeds(intervals const& is) {
  for (auto const& p : is.p_) {
    if (p.has_signal(type::MAIN_SIGNAL)) CHECK(p.signal_limit_.is_zero());
  }
}

void check_zero_length_intervals(intervals const& is) {
  for (auto const [p1, p2] : utl::pairwise(is.p_)) {
    if (p1.distance_ != p2.distance_) continue;

    CHECK_EQ(p1.limit_, p2.limit_);
    // inside a zero interval the p1 signal limit can be zero
    // and the p2 signal limit not zero.
    // since the main signal at p1 enforces the signal limit of zero,
    // but an approach signal with the same mileage (hence the zero interval)
    // has a signal limit of the general speed limit
    // so signal limits in a zero interval can increase, but not decrease
    CHECK_LE(p1.signal_limit_, p2.signal_limit_);
    CHECK_EQ(p1.bwp_limit_, p2.bwp_limit_);
  }
}

void check_every_interval_has_signal_set(intervals const& is) {
  auto const first_not_ok = utls::find_if(
      is.p_, [](auto const& p) { return !p.next_signal_.has_value(); });

  // after the first not ok signal there cannot be another ok signal
  // this is since we only allow not ok signals at the end of the interval list
  // as a train run ends on a halt or a track end and not a signal
  CHECK(std::all_of(first_not_ok, std::end(is.p_),
                    [](auto&& p) { return !p.next_signal_.has_value(); }));

  auto const first_ok = utls::find_if(
      is.p_, [](auto const& p) { return p.last_signal_.has_value(); });
  // before the first signal last_signal_ must be empty
  // but afterwards every last_signal_ must be set
  CHECK(std::all_of(first_ok, std::end(is.p_),
                    [](auto&& p) { return p.last_signal_.has_value(); }));
}

void check_signal_dists(intervals const& is) {
  for (auto const& p : is.p_) {
    CHECK(
        (!p.last_signal_.has_value() || p.last_signal_->dist_ <= p.distance_));
    CHECK(
        (!p.next_signal_.has_value() || p.next_signal_->dist_ >= p.distance_));
  }
}

void check_no_double_signals_in_zero_intervals(intervals const& is) {
  for (auto const [p1, p2] : utl::pairwise(is.p_)) {
    if (p1.distance_ != p2.distance_) continue;
    if (!p1.last_signal_.has_value() || !p2.last_signal_.has_value()) continue;
    CHECK_NE(p1.last_signal_, p2.last_signal_);
  }
}

void check_every_signal_has_interval_border(intervals const& is) {
  std::set<element::id> all_signals;
  std::set<element::id> signals_with_interval_border;

  for (auto const& p : is.p_) {
    if (p.last_signal_.has_value()) all_signals.insert(p.last_signal_->id_);
    if (p.next_signal_.has_value()) all_signals.insert(p.next_signal_->id_);

    if (p.last_signal_.has_value() && p.last_signal_->dist_ == p.distance_) {
      signals_with_interval_border.insert(p.last_signal_->id_);
    }
  }

  CHECK_EQ(all_signals, signals_with_interval_border);

  std::set<element::id> intervals_with_start_signal;
  std::set<element::id> intervals_with_end_signal;
  for (auto const& i : is) {
    if (i.starts_on_signal()) {
      intervals_with_start_signal.insert(i.start_signal().id_);
    }
    if (i.ends_on_signal()) {
      intervals_with_end_signal.insert(i.end_signal().id_);
    }
  }

  CHECK_EQ(intervals_with_start_signal, intervals_with_end_signal);
  CHECK_EQ(all_signals, intervals_with_start_signal);
}

void check_interval_signals(intervals const& is) {
  check_every_interval_has_signal_set(is);
  check_signal_dists(is);
  check_every_signal_has_interval_border(is);
  check_signal_speeds(is);
  check_no_double_signals_in_zero_intervals(is);
}

void check_interval_speed_limits(intervals const& is) {
  for (auto const [p1, p2] : utl::pairwise(is.p_)) {
    interval const interval{&p1, &p2};

    CHECK(p1.limit_.is_valid());
    CHECK_GE(p1.limit_, si::speed::zero());

    CHECK(p1.bwp_limit_.is_valid());
    CHECK_GE(p1.bwp_limit_, si::speed::zero());

    CHECK(p1.signal_limit_.is_valid());
    CHECK_GE(p1.signal_limit_, si::speed::zero());

    CHECK(p2.limit_.is_valid());
    CHECK_GE(p2.limit_, si::speed::zero());

    CHECK(p2.bwp_limit_.is_valid());
    CHECK_GE(p2.bwp_limit_, si::speed::zero());

    CHECK(p2.signal_limit_.is_valid());
    CHECK_GE(p2.signal_limit_, si::speed::zero());

    CHECK(interval.speed_limit().is_valid());
    CHECK_GE(interval.speed_limit(), si::speed::zero());

    CHECK(interval.target_speed().is_valid());
    CHECK_GE(interval.target_speed(), si::speed::zero());

    CHECK(interval.signal_limit().is_valid());
    CHECK_GE(interval.signal_limit(), si::speed::zero());

    CHECK(interval.target_signal_speed().is_valid());
    CHECK_GE(interval.target_signal_speed(), si::speed::zero());
  }
}

void check_interval_list(infrastructure const& infra, train const& train) {
  type_set const record_types({type::HALT, type::EOTD, type::MAIN_SIGNAL});

  auto const& intervals = get_intervals(train, record_types, infra);

  check_zero_length_intervals(intervals);
  check_interval_signals(intervals);
  check_interval_speed_limits(intervals);

  CHECK_MESSAGE((intervals.p_.back().distance_ == train.path_length(infra)),
                "Last interval length should be the same length as total train"
                "path length");

  for (auto const& interval : intervals) {
    CHECK_MESSAGE(interval.speed_limit().is_valid(),
                  "Found interval with speed limit 0!");
    CHECK_MESSAGE(interval.target_speed().is_valid(),
                  "Found interval with speed limit 0!");
    CHECK_MESSAGE((!interval.speed_limit().is_zero()),
                  "Found interval with speed limit 0!");
    CHECK_MESSAGE(
        (interval.ends_on_stop() || !interval.target_speed().is_zero()),
        "Found interval with speed limit 0!");

    CHECK_MESSAGE(interval.slope().is_valid(), "Found non valid slope!");

    CHECK_NE(interval.brake_path(), brake_path::invalid());

    CHECK_MESSAGE(interval.start_distance().is_valid(),
                  "Found non valid distance");
    CHECK_MESSAGE(interval.end_distance().is_valid(),
                  "Found non valid distance");

    CHECK_MESSAGE((interval.start_distance() <= interval.end_distance()),
                  "Found non increasing distance");

    if (interval.ends_on_stop()) {
      CHECK_MESSAGE(interval.sequence_point().has_value(),
                    "Halt interval needs sequence point.");
      CHECK_MESSAGE((*interval.sequence_point())->is_stop(),
                    "Halt interval needs halt sequence point.");
    }

    for (auto const& r : interval.records()) {
      CHECK_MESSAGE((r.dist_ <= interval.end_distance()),
                    "Event distance is larger than interval distance!");
      CHECK_MESSAGE((r.dist_ >= interval.start_distance()),
                    "Event distance smaller than last distance!");
    }

    for (auto const [e1, e2] : utl::pairwise(interval.records())) {
      CHECK_MESSAGE((e1.dist_ <= e2.dist_), "Event distance decreasing");
    }
  }

  std::map<type, soro::size_t> total_path_count;
  std::map<type, soro::size_t> intervals_count;

  for (auto const& type : record_types) {
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

  auto const count_interval_records = [&](auto&& records) {
    for (auto const& record : records) {
      auto const it = intervals_count.find(record.node_->element_->type());
      if (it != std::cend(intervals_count)) {
        intervals_count[record.node_->element_->type()] += 1;
      }
    }
  };

  for (auto const& interval : intervals) {
    count_interval_records(interval.records());
  }
  count_interval_records(intervals.last_records());

  for (auto const& type : record_types) {
    CHECK_MESSAGE((intervals_count[type] == total_path_count[type]),
                  "Event type count in intervals must be equal "
                  "than total path records!");
  }
}

void check_interval_list(infrastructure const& infra, timetable const& tt) {
  for (auto const& train : tt->trains_) {
    check_interval_list(infra, train);
  }
}

TEST_CASE("interval iss") {
  infrastructure const infra(DE_ISS_OPTS);
  timetable const tt(DE_KSS_OPTS, infra);

  check_interval_list(infra, tt);
}

TEST_CASE("interval") {
  for (auto const& scenario : soro::test::get_timetable_scenarios()) {
    check_interval_list(*scenario->infra_, scenario->timetable_);
  }
}

TEST_CASE("interval hill") {
  infrastructure const infra(HILL_OPTS);
  timetable const tt(HILL_TT_OPTS, infra);

  check_interval_list(infra, tt->trains_[0]);
  check_interval_list(infra, tt->trains_[1]);
  check_interval_list(infra, tt->trains_[2]);
  // train 3 throws intentionally, don't check it
  // check_interval_list(infra, tt->trains_[3]);
}

TEST_CASE("interval intersection") {
  infrastructure const infra(INTER_OPTS);
  timetable const tt(INTER_TT_OPTS, infra);

  check_interval_list(infra, tt->trains_[0]);
  check_interval_list(infra, tt->trains_[1]);
  check_interval_list(infra, tt->trains_[2]);
  // train 3 throws intentionally, don't check it
  // check_interval_list(infra, tt->trains_[3]);
}

TEST_CASE("interval follow") {
  infrastructure const infra(SMALL_OPTS);
  timetable const tt(FOLLOW_OPTS, infra);

  check_interval_list(infra, tt);
}

TEST_CASE("interval cross") {
  infrastructure const infra(SMALL_OPTS);
  timetable const tt(CROSS_OPTS, infra);

  check_interval_list(infra, tt);
}

}  // namespace soro::runtime::test
