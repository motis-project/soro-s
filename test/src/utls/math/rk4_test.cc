#include "doctest/doctest.h"

#include <iostream>

#include "utl/timer.h"

#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/runtime/common/use_surcharge.h"
#include "soro/runtime/euler_runtime.h"
#include "soro/runtime/rk4_runtime.h"

#include "test/file_paths.h"

namespace soro::test {

using namespace soro::tt;
using namespace soro::rs;
using namespace soro::infra;
using namespace soro::runtime;

TEST_CASE("delta_t test") {
  infrastructure const infra(HILL_OPTS);
  timetable const tt(HILL_TT_OPTS, infra);

  CHECK_EQ(tt->trains_.size(), 4);

  auto const duration = [](auto&& times) {
    return (times.back().arrival_ - times.front().departure_).count();
  };

  SUBCASE("sufficient brake weight percentage") {
    auto const& train = tt->trains_[0];
    CHECK_EQ(train.physics_.percentage(), rs::brake_weight_percentage{150});

    {
      utl::scoped_timer const timer("euler");
      auto const results = euler::runtime_calculation(
          train, infra, {type::HALT}, use_surcharge::no);
      CHECK_EQ(duration(results.times_), 428);
    }

    {
      utl::scoped_timer const timer("rk4");
      auto const result2 = rk4::runtime_calculation(train, infra, {type::HALT},
                                                    use_surcharge::no);
    }
  }

  //  SUBCASE("insufficient brake weight percentage - reduced max speed on
  //  slope") {
  //    auto const& train = tt->trains_[1];
  //    CHECK_EQ(train.physics_.percentage(), rs::brake_weight_percentage{67});
  //
  //    auto const results = runtime_calculation(train, infra, {type::HALT});
  //
  //    auto const result2 =
  //        runtime2(train, infra, {type::HALT}, use_surcharge::no);
  //
  //    CHECK_EQ(duration(results.times_), 468);
  //  }

  //  SUBCASE("insufficient brake weight percentage - always lowered max speed")
  //  {
  //    auto const& train = tt->trains_[2];
  //    CHECK_EQ(train.physics_.percentage(), rs::brake_weight_percentage{40});
  //
  //    auto const results = euler::runtime_calculation(train, infra,
  //    {type::HALT}); auto const result2 =
  //        rk4::runtime_calculation(train, infra, {type::HALT},
  //        use_surcharge::no);
  //
  //    CHECK_EQ(duration(results.times_), 802);
  //  }

  //  SUBCASE("insufficient brake weight percentage - not possible") {
  //    auto const& train = tt->trains_[3];
  //    CHECK_EQ(train.physics_.percentage(), rs::brake_weight_percentage{30});
  //
  //    CHECK_THROWS(euler::runtime_calculation(train, infra, {type::HALT}));
  //  }
}

TEST_CASE("rk4 test") {
  auto opts = soro::test::INTER_OPTS;
  auto tt_opts = soro::test::INTER_TT_OPTS;

  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.layout_ = false;

  infrastructure const infra(opts);
  timetable const tt(tt_opts, infra);

  //  runtime::ok(tt->trains_.front().physics_);

  for (auto const& train : tt->trains_) {
    auto const euler1 = euler::runtime_calculation(train, infra, {type::HALT},
                                                   use_surcharge::no);
    auto const euler2 = euler::runtime_calculation(train, infra, {type::HALT},
                                                   use_surcharge::yes);
    auto const rk41 =
        rk4::runtime_calculation(train, infra, {type::HALT}, use_surcharge::no);
    auto const rk42 = rk4::runtime_calculation(train, infra, {type::HALT},
                                               use_surcharge::yes);

    std::cout << "results: " << euler1.times_.size() << '\n';
    std::cout << "results2: " << rk41.times_.size() << '\n';
  }

  //   auto const& t = tt->trains_.front();
  //
  //   utl::manual_timer timer("RR Speed");
  //   auto const rr_speed =
  //       accelerate(t.physics_, si::speed::zero(), t.physics_.max_speed(),
  //                  si::from_km(10.0), si::slope::zero());
  //   //  auto const rr_speed = brake(t.physics_.max_speed(),
  //   si::speed::zero(),
  //   //                              t.physics_.default_deacceleration());
  //   timer.print("finished");
  //
  //   std::cout << "Delta T Speed Results:\n";
  //   std::cout << rr_speed.back().speed_ << std::endl;
  //   std::cout << rr_speed.back().dist_ << std::endl;
  //   std::cout << rr_speed.back().time_ << std::endl;
  //
  //   utl::manual_timer rk4_speed_timer("RK4 Speed");
  //   //  auto const rk4_speed_result =
  //   //      accelerate2(si::speed::zero(), t.physics_.max_speed(),
  //   //      si::from_km(10.0),
  //   //                  si::slope::zero(), t.physics_);
  //   //  auto const rk4_speed_result =
  //   //      brake2(t.physics_.max_speed(), si::speed::zero(),
  //   //             t.physics_.default_deacceleration());
  //   auto const rk4_speed_result =
  //       coast(t.physics_.max_speed(), si::from_m(1000.0), t.physics_,
  //             si::slope::zero());
  //   rk4_speed_timer.print("finished");
  //   std::cout << "RK4 Speed Results:\n";
  //   std::cout << "Speed: " << rk4_speed_result.speed_ << std::endl;
  //   std::cout << "Dist: " << rk4_speed_result.dist_ << std::endl;
  //   std::cout << "Time: " << rk4_speed_result.time_ << std::endl;
  //
  //   utl::manual_timer rr_dist_timer("RR Dist");
  //   auto const rr_dist =
  //       accelerate(t.physics_, si::speed::zero(), t.physics_.max_speed(),
  //                  si::from_m(100.0), si::slope::zero());
  //   rr_dist_timer.print("finished");
  //
  //   std::cout << "Delta T Dist Results:\n";
  //   std::cout << "Speed: " << rr_dist.back().speed_ << std::endl;
  //   std::cout << "Dist: " << rr_dist.back().dist_ << std::endl;
  //   std::cout << "Time: " << rr_dist.back().time_ << std::endl;
  //
  //   utl::manual_timer rk4_dist_timer("RK4 Dist");
  //   auto const rk4_dist_result =
  //       accelerate2(si::speed::zero(), t.physics_.max_speed(),
  //       si::from_m(100.0),
  //                   si::slope::zero(), t.physics_);
  //   rk4_dist_timer.print("finished");
  //   std::cout << "RK4 Dist Results:\n";
  //   std::cout << "Speed: " << rk4_dist_result.speed_ << std::endl;
  //   std::cout << "Dist: " << rk4_dist_result.dist_ << std::endl;
  //   std::cout << "Time: " << rk4_dist_result.time_ << std::endl;
}

}  // namespace soro::test
