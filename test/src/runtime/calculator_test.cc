#include "doctest/doctest.h"

#include <utility>
#include <vector>

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/narrow.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/runtime/common/event.h"
#include "soro/runtime/common/signal_time.h"
#include "soro/runtime/common/timestamps.h"
#include "soro/runtime/common/use_surcharge.h"
#include "soro/runtime/rk4_runtime.h"
#include "soro/runtime/runtime_calculator.h"

#include "test/file_paths.h"

namespace soro::runtime::test {

using namespace utl;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;
using namespace soro::test;

TEST_SUITE("calculator suite") {

  std::pair<std::vector<element::id>, std::vector<element::id>>
  get_approach_and_main(train const& train, infrastructure const& infra) {
    std::vector<element::id> main_signals;
    std::vector<element::id> approach_signals{element::invalid()};

    for (auto const& tn : train.iterate(infra)) {
      if (tn.omitted()) continue;

      switch (tn.node_->type()) {
        case type::MAIN_SIGNAL: {
          main_signals.emplace_back(tn.node_->element_->get_id());
          break;
        }
        case type::APPROACH_SIGNAL: {
          approach_signals.emplace_back(tn.node_->element_->get_id());
          break;
        }
        default: break;
      }
    }

    main_signals.emplace_back(element::invalid());
    approach_signals.emplace_back(element::invalid());

    return {approach_signals, main_signals};
  }

  void check_trip(calculator & calc, tt::train::trip const& trip,
                  infrastructure const& infra, timetable const& tt,
                  type_set const& record_types) {
    auto const& train = tt->trains_[trip.train_id_];

    timestamps ts;

    auto const expected =
        rk4::runtime_calculation(train, infra, record_types, use_surcharge::no);

    auto const event_cb1 = [&ts](event const& e) {
      ts.type_indices_[e.element_->type()].push_back(
          utls::narrow<soro::size_t>(ts.times_.size()));

      ts.times_.emplace_back(e);
    };

    auto const& [approach_signals, main_signals] =
        get_approach_and_main(train, infra);

    auto idx = 0U;
    while (!calc.finished(trip)) {
      auto const st = signal_time{
          .approach_ = approach_signals[idx],
          .main_ = main_signals[idx],
          .time_ = ZERO<absolute_time>,
      };

      auto const terminate_cb = [&](node::ptr const n) {
        return n->element_->get_id() == st.main_;
      };

      calc(trip, tt, st, event_cb1, terminate_cb);

      ++idx;
    }

    CHECK_EQ(idx, main_signals.size());
    CHECK((ts.times_ == expected.times_));
    CHECK_EQ(ts.type_indices_, expected.type_indices_);
  }

  TEST_CASE("calculator cross") {
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(CROSS_OPTS, infra);

    soro::vector<train::trip> trips;
    for (auto const& train : tt->trains_) {
      trips.emplace_back(train.trips(train::trip::id{trips.size()}).front());
    }

    type_set const record_types{type::HALT, type::APPROACH_SIGNAL,
                                type::MAIN_SIGNAL};
    calculator calc(trips, infra, tt, record_types);

    for (auto const& trip : trips) {
      check_trip(calc, trip, infra, tt, record_types);
    }
  }

  TEST_CASE("calculator follow") {
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(FOLLOW_OPTS, infra);

    soro::vector<train::trip> trips;
    for (auto const& train : tt->trains_) {
      trips.emplace_back(train.trips(train::trip::id{trips.size()}).front());
    }

    type_set const record_types{type::HALT, type::APPROACH_SIGNAL,
                                type::MAIN_SIGNAL};
    calculator calc(trips, infra, tt, record_types);

    for (auto const& trip : trips) {
      check_trip(calc, trip, infra, tt, record_types);
    }
  }
}

}  // namespace soro::runtime::test
