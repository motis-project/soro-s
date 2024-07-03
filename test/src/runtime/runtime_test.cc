#include "doctest/doctest.h"

#include <ranges>

#include "utl/logging.h"
#include "utl/parallel_for.h"

#include "soro/utls/cumulative_timer.h"
#include "soro/utls/print_progress.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/count_if.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/timetable.h"

#include "soro/runtime/common/use_surcharge.h"
#include "soro/runtime/euler_runtime.h"
#include "soro/runtime/rk4_runtime.h"

#include "test/file_paths.h"

namespace soro::runtime::test {

using namespace utl;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;
using namespace soro::test;

TEST_SUITE("runtime suite") {

  void check_halt_count(train const& t, timestamps const& ts) {
    auto const timestamps_halt_count = utls::count_if(
        ts.times_, [](auto&& stamp) { return stamp.element_->is(type::HALT); });

    auto const train_halt_count = t.total_halts();

    // There should be as many halt events in the timestamps as there are halts
    // in the train run.
    CHECK_EQ(train_halt_count, timestamps_halt_count);
  }

  void check_ascending_timestamps(timestamps const& ts) {
    if (ts.times_.empty()) {
      uLOG(utl::warn) << "Checking ascending timestamps with empty timestamps.";
      return;
    }

    for (auto const [t1, t2] : utl::pairwise(ts.times_)) {
      // all times are valid
      CHECK(valid(t1.arrival_));
      CHECK(valid(t1.departure_));
      CHECK(valid(t2.arrival_));
      CHECK(valid(t2.departure_));

      // arrival must happen before/same time as departure
      CHECK_LE(t1.arrival_, t1.departure_);

      // times must be ordered
      CHECK_LE(t1.departure_, t2.arrival_);
    }
  }

  void check_delays(infrastructure const& infra, timetable const& tt) {
    uLOG(utl::info) << "Checking delays";
    soro::size_t too_early_count = 0;
    soro::size_t delayed_count = 0;
    soro::size_t total_count = 0;

    duration max_delay = duration::zero();
    duration max_too_early = duration::zero();

    auto avg_too_early = duration::zero();
    auto avg_delay = duration::zero();

    for (auto const& train : tt->trains_) {
      auto const timestamps = euler::runtime_calculation(
          train, infra, {type::HALT}, use_surcharge::no);

      if (timestamps.times_.empty()) {
        continue;
      }

      soro::size_t halt_id = 0;
      for (auto const& sp : train.sequence_points_) {
        if (!sp.is_halt()) {
          continue;
        }

        // halts must have arrival/departure times
        CHECK(sp.arrival_.has_value());
        CHECK(sp.departure_.has_value());

        auto const& ts =
            timestamps.times_[timestamps.type_indices_.at(type::HALT)[halt_id]];

        ++total_count;
        if (valid(ts.departure_) && ts.departure_ > *sp.departure_) {
          ++delayed_count;
          auto const delay = ts.departure_ - *sp.departure_;
          max_delay = std::max(max_delay, delay);
          avg_delay += ts.departure_ - *sp.departure_;
        }

        if (valid(ts.arrival_) && ts.arrival_ < *sp.arrival_) {
          ++too_early_count;
          auto const too_early = *sp.arrival_ - ts.arrival_;
          max_too_early = std::max(max_too_early, too_early);
          avg_too_early += *sp.arrival_ - ts.arrival_;
        }

        ++halt_id;
      }
    }

    uLOG(info) << "Total halt timestamps: " << total_count;

    uLOG(info) << "Total delayed timestamps: " << delayed_count;
    uLOG(info) << "Total over punctual timestamps: " << too_early_count;

    if (avg_delay != duration::zero()) {
      uLOG(info) << "AVG delay: " << avg_delay.count() / delayed_count;
    }

    if (avg_too_early != duration::zero()) {
      uLOG(info) << "AVG too early: "
                 << avg_too_early.count() / too_early_count;
    }

    uLOG(info) << "Maximum delay: " << max_delay.count();
    uLOG(info) << "Maximum over punctuality: " << max_too_early.count();
  }

  void check_timestamps_type_indices(timestamps const& ts) {
    for (auto const& [type, indices] : ts.type_indices_) {
      // every index given in type_indices_ must fit the type in the timestamps
      CHECK(utls::all_of(indices, [&ts, &type](auto&& idx) {
        return ts.times_[idx].element_->is(type);
      }));

      // every timestamp type must appear in type_indices_
      CHECK(utls::all_of(ts.times_, [&ts](auto&& t) {
        return ts.type_indices_.contains(t.element_->type());
      }));
    }
  }

  void check_event_existence_in_timestamps(train const& t, timestamps const& ts,
                                           infrastructure const& infra) {
    type_set const event_types(std::views::keys(ts.type_indices_));

    soro::size_t ts_idx = 0;
    for (auto const& rn : t.iterate(infra)) {
      if (rn.omitted() || !event_types.contains(rn.node_->type())) continue;

      bool const same_element =
          ts.times_[ts_idx].element_->get_id() == rn.node_->element_->get_id();

      CHECK_MESSAGE(
          same_element,
          "element from timestamps does not correspond to train run element");

      ++ts_idx;
    }

    CHECK_MESSAGE((ts_idx == ts.times_.size()),
                  "did not check every timestamp");
  }

  void determine_runtime_performance(infrastructure const& infra,
                                     timetable const& tt,
                                     type_set const& record_types) {
    utls::cumulative_timer euler_timer;
    utls::cumulative_timer rk4_timer;

    for (auto const& t : tt->trains_) {
      rk4_timer.start();
      auto const ts =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::no);
      rk4_timer.stop();

      euler_timer.start();
      std::ignore =
          euler::runtime_calculation(t, infra, record_types, use_surcharge::no);
      euler_timer.stop();
    }

    CHECK_EQ(euler_timer.count(), rk4_timer.count());

    uLOG(info) << "Calculated " << rk4_timer.count() << " trains";
    uLOG(info) << "Average RK4 Time: " << rk4_timer.avg_duration()
               << "microseconds";
    uLOG(info) << "Average Euler Time: " << euler_timer.avg_duration()
               << "microseconds";
  }

  void check_runtime_with_events(infrastructure const& infra,
                                 timetable const& tt,
                                 type_set const& record_types) {
    for (auto const& t : tt->trains_) {
      auto const rk4_result =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::no);

      auto const euler_result =
          euler::runtime_calculation(t, infra, record_types, use_surcharge::no);

      if (record_types.contains(type::HALT)) {
        check_halt_count(t, euler_result);
        check_halt_count(t, rk4_result);
      }

      check_timestamps_type_indices(euler_result);
      check_ascending_timestamps(euler_result);
      check_event_existence_in_timestamps(t, euler_result, infra);

      check_timestamps_type_indices(rk4_result);
      check_ascending_timestamps(rk4_result);
      check_event_existence_in_timestamps(t, rk4_result, infra);
    }
  }

  void check_runtime(infrastructure const& infra, timetable const& tt) {
    auto const only_halt = type_set{type::HALT};
    check_runtime_with_events(infra, tt, only_halt);

    auto const simulation_types =
        type_set{type::RUNTIME_CHECKPOINT_UNDIRECTED, type::RUNTIME_CHECKPOINT,
                 type::APPROACH_SIGNAL, type::MAIN_SIGNAL, type::EOTD};
    check_runtime_with_events(infra, tt, simulation_types);

    check_runtime_with_events(infra, tt, type_set{all_types()});
  }

  void determine_runtime_performance(infrastructure const& infra,
                                     timetable const& tt) {
    uLOG(info) << "runtime performance while recording halts:";
    determine_runtime_performance(infra, tt, {type::HALT});
    uLOG(info) << "runtime performance while recording all types:";
    determine_runtime_performance(infra, tt, type_set{all_types()});
  }

  TEST_CASE("runtime") {
    for (auto const& scenario : soro::test::get_timetable_scenarios()) {
      check_runtime(*scenario->infra_, scenario->timetable_);
      check_delays(*scenario->infra_, scenario->timetable_);

      // when in debug mode skip runtime performance
#if defined(NDEBUG)
      determine_runtime_performance(*scenario->infra_, scenario->timetable_);
#endif
    }
  }

  constexpr auto runs = 100U;

  void test_euler(infrastructure const& infra, timetable const& tt) {
    for (auto i = 0U; i < runs; ++i) {
      for (auto const& t : tt->trains_) {
        if (t.id_ >= 1000) continue;

        auto const record_types = type_set{
            type::RUNTIME_CHECKPOINT_UNDIRECTED, type::RUNTIME_CHECKPOINT,
            type::APPROACH_SIGNAL, type::MAIN_SIGNAL, type::EOTD};

        auto const euler_result = euler::runtime_calculation(
            t, infra, record_types, use_surcharge::no);
      }
    }
  }

  void test_rk4(infrastructure const& infra, timetable const& tt) {
    for (auto i = 0U; i < runs; ++i) {
      for (auto const& t : tt->trains_) {
        if (t.id_ >= 1000) continue;

        auto const record_types = type_set{
            type::RUNTIME_CHECKPOINT_UNDIRECTED, type::RUNTIME_CHECKPOINT,
            type::APPROACH_SIGNAL, type::MAIN_SIGNAL, type::EOTD};

        auto const rk4_result =
            rk4::runtime_calculation(t, infra, record_types, use_surcharge::no);
      }
    }
  }

  TEST_CASE("runtime iss") {
    auto const infra = utls::try_deserializing<infrastructure>(
        "de_iss_runtime.raw", DE_ISS_OPTS);
    auto const tt = utls::try_deserializing<timetable>("de_kss_runtime.raw",
                                                       DE_KSS_OPTS, infra);

    test_euler(infra, tt);
    test_rk4(infra, tt);

    //    check_runtime(infra, tt);
    //    check_delays(infra, tt);
  }

  TEST_CASE("runtime hill") {
    infrastructure const infra(HILL_OPTS);
    timetable const tt(HILL_TT_OPTS, infra);

    CHECK_EQ(tt->trains_.size(), 4);

    auto const record_types = infra::type_set{type::HALT};

    auto const duration = [](auto&& times) {
      return (times.back().arrival_ - times.front().departure_).count();
    };

    SUBCASE("sufficient brake weight percentage") {
      auto const& t = tt->trains_[0];
      CHECK_EQ(t.physics_.percentage(), rs::brake_weight_percentage{150});

      auto const euler_no =
          euler::runtime_calculation(t, infra, record_types, use_surcharge::no);
      auto const euler_yes = euler::runtime_calculation(t, infra, record_types,
                                                        use_surcharge::yes);

      auto const rk4_no =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::no);
      auto const rk4_yes =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::yes);

      CHECK_EQ(duration(euler_no.times_), 428);
      CHECK_EQ(duration(rk4_no.times_), 428);

      CHECK_EQ(duration(euler_yes.times_), 440);
      CHECK_EQ(duration(rk4_yes.times_), 440);
    }

    SUBCASE(
        "insufficient brake weight percentage - reduced max speed on slope") {
      auto const& t = tt->trains_[1];
      CHECK_EQ(t.physics_.percentage(), rs::brake_weight_percentage{67});

      auto const euler_no =
          euler::runtime_calculation(t, infra, record_types, use_surcharge::no);
      auto const euler_yes = euler::runtime_calculation(t, infra, record_types,
                                                        use_surcharge::yes);

      auto const rk4_no =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::no);
      auto const rk4_yes =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::yes);

      CHECK_EQ(duration(euler_no.times_), 468);
      CHECK_EQ(duration(rk4_no.times_), 467);

      CHECK_EQ(duration(euler_yes.times_), 481);
      CHECK_EQ(duration(rk4_yes.times_), 481);
    }

    SUBCASE("insufficient brake weight percentage - always lowered max speed") {
      auto const& t = tt->trains_[2];
      CHECK_EQ(t.physics_.percentage(), rs::brake_weight_percentage{40});

      auto const euler_no =
          euler::runtime_calculation(t, infra, record_types, use_surcharge::no);
      auto const euler_yes = euler::runtime_calculation(t, infra, record_types,
                                                        use_surcharge::yes);

      auto const rk4_no =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::no);
      auto const rk4_yes =
          rk4::runtime_calculation(t, infra, record_types, use_surcharge::yes);

      CHECK_EQ(duration(euler_no.times_), 803);
      CHECK_EQ(duration(rk4_no.times_), 802);

      CHECK_EQ(duration(euler_yes.times_), 826);
      CHECK_EQ(duration(rk4_yes.times_), 826);
    }

    SUBCASE("insufficient brake weight percentage - not possible") {
      auto const& train = tt->trains_[3];
      CHECK_EQ(train.physics_.percentage(), rs::brake_weight_percentage{30});

      CHECK_THROWS(euler::runtime_calculation(train, infra, {type::HALT},
                                              use_surcharge::no));
      CHECK_THROWS(rk4::runtime_calculation(train, infra, {type::HALT},
                                            use_surcharge::no));
    }
  }

  TEST_CASE("runtime intersection") {
    infrastructure const infra(INTER_OPTS);
    timetable const tt(INTER_TT_OPTS, infra);

    CHECK_EQ(tt->trains_.size(), 4);

    auto const record_types = infra::type_set{type::HALT};

    auto const duration = [](auto&& times) {
      return (times.back().arrival_ - times.front().departure_).count();
    };

    auto const& t = tt->trains_[0];
    CHECK_EQ(t.physics_.percentage(), rs::brake_weight_percentage{150});

    auto const euler_no =
        euler::runtime_calculation(t, infra, record_types, use_surcharge::no);
    auto const euler_yes =
        euler::runtime_calculation(t, infra, record_types, use_surcharge::yes);

    auto const rk4_no =
        rk4::runtime_calculation(t, infra, record_types, use_surcharge::no);
    auto const rk4_yes =
        rk4::runtime_calculation(t, infra, record_types, use_surcharge::yes);

    CHECK_EQ(duration(euler_no.times_), 506);
    CHECK_EQ(duration(rk4_no.times_), 505);

    CHECK_EQ(duration(euler_yes.times_), 522);
    CHECK_EQ(duration(rk4_yes.times_), 520);
  }

  TEST_CASE("runtime follow") {
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(FOLLOW_OPTS, infra);

    check_runtime(infra, tt);
    check_delays(infra, tt);
  }

  TEST_CASE("runtime cross") {
    auto const infra =
        utls::try_deserializing<infrastructure>("small_opts.raw", SMALL_OPTS);
    auto const tt =
        utls::try_deserializing<timetable>("cross_opts.raw", CROSS_OPTS, infra);

    check_runtime(infra, tt);
    check_delays(infra, tt);
  }
}

}  // namespace soro::runtime::test
