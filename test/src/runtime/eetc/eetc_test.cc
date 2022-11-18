#include "doctest/doctest.h"

#include <chrono>

#include "soro/rolling_stock/train_series.h"
#include "soro/si/constants.h"
#include "soro/timetable/timetable.h"

#include "soro/runtime/driving_regimes/physics.h"
#include "soro/runtime/eetc/sheepmaker.h"
#include "soro/runtime/eetc/speed_interval.h"
#include "soro/runtime/interval.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::rs;
using namespace soro::infra;
using namespace soro::runtime;

using namespace std::chrono;

rs::train_physics generate_frictionless_test_train_2() {
  soro::vector<tractive_piece_t> polynomials;

  tractive_polynomial_t const polynomial = utls::make_polynomial(
      tractive_force_3_t{1000.0F}, tractive_force_2_t{10000.0F},
      tractive_force_1_t{100000.0F});
  tractive_piece_t const piece =
      utls::make_piece(polynomial, si::speed{0.0F}, si::speed{300.0F});

  polynomials.push_back(piece);
  tractive_curve_t const tractive_piece_poly =
      utls::make_piecewise(std::move(polynomials));

  drag_coefficient_t drag{0.0F};
  dampening_resistance_t dampening{0.0F};
  rolling_resistance_t rolling{0.0F};
  resistance_curve_t const resistance_poly =
      utls::make_polynomial(drag, dampening, rolling);

  return {{traction_vehicle{.name_ = "FRICTIONLESS_TEST_TRAIN",
                            .weight_ = si::from_kg(400000.0),
                            .max_speed_ = si::from_km_h(62.0),
                            .deacceleration_ = si::from_m_s2(-1.5),
                            .tractive_curve_ = tractive_piece_poly,
                            .resistance_curve_ = resistance_poly}},
          si::from_kg(0.0),
          si::from_m(0.0),
          si::from_km_h(400.0)};
}

rs::train_physics generate_test_train_2() {
  soro::vector<tractive_piece_t> polynomials;

  tractive_polynomial_t const polynomial = utls::make_polynomial(
      tractive_force_3_t{1000.0F}, tractive_force_2_t{10000.0F},
      tractive_force_1_t{100000.0F});
  tractive_piece_t const piece =
      utls::make_piece(polynomial, si::speed{0.0F}, si::speed{300.0F});

  polynomials.push_back(piece);
  tractive_curve_t const tractive_piece_poly =
      utls::make_piecewise(std::move(polynomials));

  drag_coefficient_t drag{(0.00005F / 1000.0F) * si::GRAVITATIONAL.val_ * 3.6 *
                          3.6};
  dampening_resistance_t dampening{0.0005F * si::GRAVITATIONAL.val_ *
                                   si::weight{400000.0F}.val_ * 3.6};
  rolling_resistance_t rolling{0.005F * si::GRAVITATIONAL.val_ *
                               si::weight{400000.0F}.val_};
  resistance_curve_t const resistance_poly =
      utls::make_polynomial(drag, dampening, rolling);

  return {{traction_vehicle{.name_ = "TEST_TRAIN",
                            .weight_ = si::from_kg(400000.0F),
                            .max_speed_ = si::from_km_h(62.0F),
                            .deacceleration_ = si::from_m_s2(-1.5F),
                            .tractive_curve_ = tractive_piece_poly,
                            .resistance_curve_ = resistance_poly}},
          si::from_kg(0.0),
          si::from_m(0.0),
          si::from_km_h(400.0)};
}

void check_speed_intervals(speed_intervals is_speed_intervals,
                           speed_intervals shall_speed_intervals) {
  CHECK(is_speed_intervals.size() == shall_speed_intervals.size());

  for (std::size_t i = 0UL; i < is_speed_intervals.size(); ++i) {
    CHECK(is_speed_intervals[i].speed_limit_ ==
          shall_speed_intervals[i].speed_limit_);
    CHECK(is_speed_intervals[i].length() == shall_speed_intervals[i].length());
    CHECK(is_speed_intervals[i].start_ == shall_speed_intervals[i].start_);
    CHECK(is_speed_intervals[i].end_ == shall_speed_intervals[i].end_);
    CHECK(is_speed_intervals[i].halt_ == shall_speed_intervals[i].halt_);

    CHECK(is_speed_intervals[i].intervals_.size() ==
          shall_speed_intervals[i].intervals_.size());
    for (std::size_t j = 0UL; j < is_speed_intervals[i].intervals_.size();
         ++j) {
      CHECK(is_speed_intervals[i].intervals_[j].speed_limit_ ==
            shall_speed_intervals[i].intervals_[j].speed_limit_);
      CHECK(is_speed_intervals[i].intervals_[j].distance_ ==
            shall_speed_intervals[i].intervals_[j].distance_);
      CHECK(is_speed_intervals[i].intervals_[j].halt_ ==
            shall_speed_intervals[i].intervals_[j].halt_);
    }
  }
}

TEST_SUITE_BEGIN("speed_intervals");  // NOLINT

TEST_CASE("speed_interval::has_driving_regime_type::empty") {  // NOLINT
  interval_list const intervals = {
      interval(si::length{0.0F}, si::speed{10.0F})};
  auto test_speed_interval = speed_interval{si::length{0.0F}, si::length{10.0F},
                                            si::speed{10.0F}, intervals};

  CHECK(!test_speed_interval.has_driving_regime_type(MAX_ACCELERATION));
  CHECK(!test_speed_interval.has_driving_regime_type(CRUISING));
  CHECK(!test_speed_interval.has_driving_regime_type(COASTING));
  CHECK(!test_speed_interval.has_driving_regime_type(MAX_BRAKING));
}

TEST_CASE("speed_interval::has_driving_regime_type::acc") {  // NOLINT
  interval_list const intervals = {
      interval(si::length{0.0F}, si::speed{10.0F})};
  auto test_speed_interval = speed_interval{si::length{0.0F}, si::length{10.0F},
                                            si::speed{10.0F}, intervals};
  auto accelerate =
      general_driving(si::length{0.0F}, si::length{5.0F}, si::speed{0.0F},
                      si::time{0.0F}, MAX_ACCELERATION);

  test_speed_interval.driving_regimes_.emplace_back(accelerate);

  CHECK(test_speed_interval.has_driving_regime_type(MAX_ACCELERATION));
  CHECK(!test_speed_interval.has_driving_regime_type(CRUISING));
  CHECK(!test_speed_interval.has_driving_regime_type(COASTING));
  CHECK(!test_speed_interval.has_driving_regime_type(MAX_BRAKING));
}

TEST_CASE("speed_interval::has_driving_regime_type::acc-cruise") {  // NOLINT
  interval_list const intervals = {
      interval(si::length{0.0F}, si::speed{10.0F})};
  auto test_speed_interval = speed_interval{si::length{0.0F}, si::length{10.0F},
                                            si::speed{10.0F}, intervals};
  auto accelerate =
      general_driving(si::length{0.0F}, si::length{5.0F}, si::speed{0.0F},
                      si::time{0.0F}, MAX_ACCELERATION);
  auto cruising = general_driving(si::length{5.0F}, si::length{7.0F},
                                  si::speed{2.0F}, si::time{1.0F}, CRUISING);

  test_speed_interval.driving_regimes_.emplace_back(accelerate);
  test_speed_interval.driving_regimes_.emplace_back(cruising);

  CHECK(test_speed_interval.has_driving_regime_type(MAX_ACCELERATION));
  CHECK(test_speed_interval.has_driving_regime_type(CRUISING));
  CHECK(!test_speed_interval.has_driving_regime_type(COASTING));
  CHECK(!test_speed_interval.has_driving_regime_type(MAX_BRAKING));
}

TEST_CASE(  // NOLINT
    "speed_interval::has_driving_regime_type::acc-cruising-coasting") {
  interval_list const intervals = {
      interval(si::length{0.0F}, si::speed{10.0F})};
  auto test_speed_interval = speed_interval{si::length{0.0F}, si::length{10.0F},
                                            si::speed{10.0F}, intervals};
  auto accelerate =
      general_driving(si::length{0.0F}, si::length{5.0F}, si::speed{0.0F},
                      si::time{0.0F}, MAX_ACCELERATION);
  auto cruising = general_driving(si::length{5.0F}, si::length{7.0F},
                                  si::speed{2.0F}, si::time{1.0F}, CRUISING);
  auto coasting = general_driving(si::length{7.0F}, si::length{8.0F},
                                  si::speed{2.0F}, si::time{1.0F}, COASTING);

  test_speed_interval.driving_regimes_.emplace_back(accelerate);
  test_speed_interval.driving_regimes_.emplace_back(cruising);
  test_speed_interval.driving_regimes_.emplace_back(coasting);

  CHECK(test_speed_interval.has_driving_regime_type(MAX_ACCELERATION));
  CHECK(test_speed_interval.has_driving_regime_type(CRUISING));
  CHECK(test_speed_interval.has_driving_regime_type(COASTING));
  CHECK(!test_speed_interval.has_driving_regime_type(MAX_BRAKING));
}

TEST_CASE(  // NOLINT
    "speed_interval::has_driving_regime_type::acc-cruising-coasting-brake") {
  interval_list const intervals = {
      interval(si::length{0.0F}, si::speed{10.0F})};
  auto test_speed_interval = speed_interval{si::length{0.0F}, si::length{10.0F},
                                            si::speed{10.0F}, intervals};
  auto accelerate =
      general_driving(si::length{0.0F}, si::length{5.0F}, si::speed{0.0F},
                      si::time{0.0F}, MAX_ACCELERATION);
  auto cruising = general_driving(si::length{5.0F}, si::length{7.0F},
                                  si::speed{2.0F}, si::time{1.0F}, CRUISING);
  auto coasting = general_driving(si::length{7.0F}, si::length{8.0F},
                                  si::speed{2.0F}, si::time{1.0F}, COASTING);
  auto braking = general_driving(si::length{8.0F}, si::length{10.0F},
                                 si::speed{1.5F}, si::time{1.0F}, MAX_BRAKING);

  test_speed_interval.driving_regimes_.emplace_back(accelerate);
  test_speed_interval.driving_regimes_.emplace_back(cruising);
  test_speed_interval.driving_regimes_.emplace_back(coasting);
  test_speed_interval.driving_regimes_.emplace_back(braking);

  CHECK(test_speed_interval.has_driving_regime_type(MAX_ACCELERATION));
  CHECK(test_speed_interval.has_driving_regime_type(CRUISING));
  CHECK(test_speed_interval.has_driving_regime_type(COASTING));
  CHECK(test_speed_interval.has_driving_regime_type(MAX_BRAKING));
}

TEST_CASE("get_speed_intervals::empty") {  // NOLINT
  auto const is_speed_intervals = get_speed_intervals({});
  CHECK(is_speed_intervals.empty());
}

TEST_CASE("get_speed_intervals::no_speed_limit_update") {  // NOLINT
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{5.0F}),
      interval(si::length{25.0F}, si::speed{5.0F}),
      interval(si::length{30.0F}, si::speed{5.0F}, true, type::HALT)};
  interval_list const first = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{5.0F}),
      interval(si::length{25.0F}, si::speed{5.0F})};

  speed_intervals const shall_speed_intervals = {speed_interval{
      si::length{0.0F}, si::length{30.0F}, si::speed{5.0F}, true, first}};
  auto const is_speed_intervals = get_speed_intervals(input);

  check_speed_intervals(is_speed_intervals, shall_speed_intervals);
}

TEST_CASE("get_speed_intervals::speed_limit_update") {  // NOLINT
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};
  interval_list const first = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F})};
  interval_list const second = {interval(si::length{15.0F}, si::speed{2.5F})};

  speed_intervals const shall_speed_intervals = {
      speed_interval{si::length{0.0F}, si::length{15.0F}, si::speed{5.0F},
                     first},
      // speed_interval(m{10.0F}, m{15.0F}, m_s{5.0F}, second),
      speed_interval(si::length{15.0F}, si::length{25.0F}, si::speed{2.5F},
                     true, second)};
  auto const is_speed_intervals = get_speed_intervals(input);

  check_speed_intervals(is_speed_intervals, shall_speed_intervals);
}

TEST_CASE("get_speed_intervals::multiple_speed_intervals") {  // NOLINT
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{15.0F}, si::speed{4.0F}),
      interval(si::length{25.0F}, si::speed{3.0F}),
      interval(si::length{30.0F}, si::speed{0.0F}, true, type::HALT)};
  interval_list const first = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT)};
  interval_list const second = {interval(si::length{15.0F}, si::speed{4.0F})};
  interval_list const third = {interval(si::length{25.0F}, si::speed{3.0F})};

  speed_intervals const shall_speed_intervals = {
      speed_interval(si::length{0.0F}, si::length{15.0F}, si::speed{5.0F},
                     first),
      speed_interval(si::length{15.0F}, si::length{25.0F}, si::speed{4.0F},
                     second),
      speed_interval(si::length{25.0F}, si::length{30.0F}, si::speed{3.0F},
                     true, third)};
  auto const is_speed_intervals = get_speed_intervals(input);

  check_speed_intervals(is_speed_intervals, shall_speed_intervals);
}

TEST_CASE("get_cruising_speed_range::base_case") {
  // BASE-CASE
  // - v_i = v_f = 0 m/s
  // - v_max = 1 m/s
  // - ACCELERATION, CRUISING at max velocity, MAX_BRAKING

  auto shall_speed_range = std::make_tuple(si::speed{0.0F}, si::speed{1.0F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{0.0F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {0.0F}),
                            runtime_result({1.0F}, {1.0F}, {1.0F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{0.0F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {1.0F}),
                        runtime_result({2.0F}, {2.0F}, {1.0F})};

  general_driving braking =
      general_driving(si::length{2.0F}, si::length{3.0F}, si::speed{0.0F},
                      si::time{0.0F}, MAX_BRAKING);
  braking.last_run_ = {runtime_result({2.0F}, {2.0F}, {1.0F}),
                       runtime_result({3.0F}, {3.0F}, {0.0F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{3.0F},
                                            si::speed{1.0F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising, braking};

  auto is_speed_range = speed_int.get_cruising_speed_range();

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_speed_range::scaleddown_base_case") {
  // BASE-CASE -- scaled down
  // - v_i = v_f = 0.25 m/s
  // - v_max = 0.75 m/s
  // - ACCELERATION, CRUISING at max velocity, MAX_BRAKING

  auto shall_speed_range = std::make_tuple(si::speed{0.25F}, si::speed{0.75F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{0.25F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {0.25F}),
                            runtime_result({1.0F}, {1.0F}, {0.75F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{0.75F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {0.75F}),
                        runtime_result({2.0F}, {2.0F}, {0.75F})};

  general_driving braking =
      general_driving(si::length{2.0F}, si::length{3.0F}, si::speed{0.75F},
                      si::time{0.0F}, MAX_BRAKING);
  braking.last_run_ = {runtime_result({2.0F}, {2.0F}, {0.75F}),
                       runtime_result({3.0F}, {3.0F}, {0.25F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{3.0F},
                                            si::speed{1.0F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising, braking};

  auto is_speed_range = speed_int.get_cruising_speed_range();

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_speed_range::lower_final_speed") {
  // - v_i = 0.5 m/s
  // - v_f = 0.25 m/s
  // - v_max = 1 m/s
  // - ACCELERATION, CRUISING at max velocity, MAX_BRAKING

  auto shall_speed_range = std::make_tuple(si::speed{0.5F}, si::speed{1.0F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{0.5F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {0.50F}),
                            runtime_result({1.0F}, {1.0F}, {1.0F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {1.0F}),
                        runtime_result({2.0F}, {2.0F}, {1.0F})};

  general_driving braking =
      general_driving(si::length{2.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_BRAKING);
  braking.last_run_ = {runtime_result({2.0F}, {2.0F}, {1.0F}),
                       runtime_result({3.0F}, {3.0F}, {0.25F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{3.0F},
                                            si::speed{1.0F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising, braking};

  auto is_speed_range = speed_int.get_cruising_speed_range();

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_speed_range::no_braking") {
  // - v_i = 0 m/s
  // - v_f = 1 m/s
  // - v_max = 1 m/s
  // - ACCELERATION, CRUISING at max velocity, MAX_BRAKING

  auto shall_speed_range = std::make_tuple(si::speed{0.0F}, si::speed{1.0F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{0.0F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {0.0F}),
                            runtime_result({1.0F}, {1.0F}, {1.0F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {1.0F}),
                        runtime_result({2.0F}, {2.0F}, {1.0F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{2.0F},
                                            si::speed{1.0F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising};

  auto is_speed_range = speed_int.get_cruising_speed_range();

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_speed_range::higher_final_speed_full_range") {
  // - v_i = 0.25 m/s
  // - v_f = 0.5 m/s
  // - v_max = 1.0 m/s
  // - ACCELERATION, CRUISING at max velocity, MAX_BRAKING

  auto shall_speed_range = std::make_tuple(si::speed{0.25F}, si::speed{1.0F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{0.25F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {0.25F}),
                            runtime_result({1.0F}, {1.0F}, {1.0F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {1.0F}),
                        runtime_result({2.0F}, {2.0F}, {1.0F})};

  general_driving braking =
      general_driving(si::length{2.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_BRAKING);
  braking.last_run_ = {runtime_result({2.0F}, {2.0F}, {1.0F}),
                       runtime_result({3.0F}, {3.0F}, {0.5F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{3.0F},
                                            si::speed{1.0F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising, braking};

  auto is_speed_range = speed_int.get_cruising_speed_range(false);

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_speed_range::higher_final_speed_safe_range") {
  // - v_i = 0.25 m/s
  // - v_f = 0.5 m/s
  // - v_max = 1.0 m/s
  // - ACCELERATION, CRUISING at max velocity, MAX_BRAKING

  auto shall_speed_range = std::make_tuple(si::speed{0.5F}, si::speed{1.0F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{0.25F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {0.25F}),
                            runtime_result({1.0F}, {1.0F}, {1.0F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {1.0F}),
                        runtime_result({2.0F}, {2.0F}, {1.0F})};

  general_driving braking =
      general_driving(si::length{2.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_BRAKING);
  braking.last_run_ = {runtime_result({2.0F}, {2.0F}, {1.0F}),
                       runtime_result({3.0F}, {3.0F}, {0.5F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{3.0F},
                                            si::speed{1.0F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising, braking};

  auto is_speed_range = speed_int.get_cruising_speed_range();

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_speed_range::edge_case_constant_speed") {
  // - v_i = v_f = v_max
  // - ACCELERATION (no change, at max), CRUISING at max velocity

  auto shall_speed_range = std::make_tuple(si::speed{0.75F}, si::speed{0.75F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{0.75F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {0.75F}),
                            runtime_result({1.0F}, {1.0F}, {0.75F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{0.75F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {0.75F}),
                        runtime_result({2.0F}, {2.0F}, {0.75F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{3.0F},
                                            si::speed{0.75F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising};

  auto is_speed_range = speed_int.get_cruising_speed_range();

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_speed_range::edge_case_only_branking_changes_speed") {
  // - v_i = v_max
  // - v_f < v_max
  // - ACCELERATION (no change, at max), CRUISING at max velocity, MAX_BRAKING

  auto shall_speed_range = std::make_tuple(si::speed{1.0F}, si::speed{1.0F});

  general_driving acceleration =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_ACCELERATION);
  acceleration.last_run_ = {runtime_result({0.0F}, {0.0F}, {1.0F}),
                            runtime_result({1.0F}, {1.0F}, {1.0F})};

  general_driving cruising =
      general_driving(si::length{1.0F}, si::length{2.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);
  cruising.last_run_ = {runtime_result({1.0F}, {1.0F}, {1.0F}),
                        runtime_result({2.0F}, {2.0F}, {1.0F})};

  general_driving braking =
      general_driving(si::length{2.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_BRAKING);
  braking.last_run_ = {runtime_result({2.0F}, {2.0F}, {1.0F}),
                       runtime_result({3.0F}, {3.0F}, {0.5F})};

  interval_list const dummy = {};
  speed_interval speed_int = speed_interval(si::length{0.0F}, si::length{3.0F},
                                            si::speed{1.0F}, dummy);
  speed_int.driving_regimes_ = {acceleration, cruising, braking};

  auto is_speed_range = speed_int.get_cruising_speed_range();

  CHECK(std::get<0>(is_speed_range) == std::get<0>(shall_speed_range));
  CHECK(std::get<1>(is_speed_range) == std::get<1>(shall_speed_range));
}

TEST_CASE("get_cruising_interval::no_braking") {  // NOLINT
  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);
  auto acc = general_driving(si::length{0.0F}, si::length{5.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto cruising = general_driving(si::length{5.0F}, si::length{10.0F},
                                  si::speed{5.0F}, si::time{5.0F}, CRUISING);

  // TEST: NO ACCELERATION
  CHECK_THROWS(speed_int.get_cruising_interval(si::speed{10.0F}));

  // TEST: NO BRAKING
  speed_int.driving_regimes_ = {acc, cruising};
  CHECK_THROWS(speed_int.get_cruising_interval(si::speed{10.0F}));
}

TEST_CASE("get_cruising_interval::candidate_out_of_range") {  // NOLINT
  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);

  auto acc = general_driving(si::length{0.0F}, si::length{5.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto braking = general_driving(si::length{5.0F}, si::length{10.0F},
                                 si::speed{3.0F}, si::time{5.0F}, MAX_BRAKING);

  runtime_results rr_acc, rr_braking;

  rr_acc.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{3.0F}, si::length{2.0F}, si::speed{1.5F});
  rr_acc.emplace_back(si::time{2.0F}, si::length{3.0F}, si::speed{2.0F});
  rr_acc.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{2.5F});
  rr_acc.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{3.0F});

  rr_braking.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{3.0F});
  rr_braking.emplace_back(si::time{6.0F}, si::length{6.0F}, si::speed{2.5F});
  rr_braking.emplace_back(si::time{7.0F}, si::length{7.0F}, si::speed{2.0F});
  rr_braking.emplace_back(si::time{8.0F}, si::length{8.0F}, si::speed{1.5F});
  rr_braking.emplace_back(si::time{9.0F}, si::length{9.0F}, si::speed{1.0F});
  rr_braking.emplace_back(si::time{10.0F}, si::length{10.0F}, si::speed{0.5F});

  // LAST_RUN ALLOCATION
  acc.last_run_ = rr_acc;
  braking.last_run_ = rr_braking;

  // ASSIGN DRIVING REGIMES
  speed_int.driving_regimes_ = {acc, braking};

  // TEST: CRUISING CANDIDATE TOO HIGH
  auto too_high_interval = speed_int.get_cruising_interval(si::speed{4.0F});

  CHECK(std::get<0>(too_high_interval) == si::length{-1.0F});
  CHECK(std::get<1>(too_high_interval) == si::length{-1.0F});

  // TEST: CRUISING CANDIDATE TOO LOW
  auto too_low_interval = speed_int.get_cruising_interval(si::speed{0.0F});

  CHECK(std::get<0>(too_low_interval) == si::length{-1.0F});
  CHECK(std::get<1>(too_low_interval) == si::length{-1.0F});
}

TEST_CASE(
    "get_cruising_interval::coasting_isp_and_no_isp_combination") {  // NOLINT
  // ISP - INTERSECTION POINT

  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);

  auto acc = general_driving(si::length{0.0F}, si::length{5.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto coasting = general_driving(si::length{5.0F}, si::length{10.0F},
                                  si::speed{3.0F}, si::time{5.0F}, COASTING);
  auto braking = general_driving(si::length{10.0F}, si::length{10.0F},
                                 si::speed{1.75}, si::time{10.0F}, MAX_BRAKING);

  runtime_results rr_acc, rr_coasting, rr_braking;

  rr_acc.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{2.0F}, si::length{2.0F}, si::speed{1.5F});
  rr_acc.emplace_back(si::time{3.0F}, si::length{3.0F}, si::speed{2.0F});
  rr_acc.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{2.5F});
  rr_acc.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{3.0F});

  rr_coasting.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{3.0F});
  rr_coasting.emplace_back(si::time{6.0F}, si::length{6.0F}, si::speed{2.75F});
  rr_coasting.emplace_back(si::time{7.0F}, si::length{7.0F}, si::speed{2.5F});
  rr_coasting.emplace_back(si::time{8.0F}, si::length{8.0F}, si::speed{2.25F});
  rr_coasting.emplace_back(si::time{9.0F}, si::length{9.0F}, si::speed{2.0F});
  rr_coasting.emplace_back(si::time{10.0F}, si::length{10.0F},
                           si::speed{1.75F});

  rr_braking.emplace_back(si::time{10.0F}, si::length{10.0F}, si::speed{1.75});

  // LAST RUN ALLOCATION
  acc.last_run_ = rr_acc;
  coasting.last_run_ = rr_coasting;
  braking.last_run_ = rr_braking;

  // ASSIGN DRIVING REGIMES
  speed_int.driving_regimes_ = {acc, coasting, braking};

  // TEST: CRUISING CANDIDATE TOO LOW: NO INTERSECTION POINT
  auto too_low_interval = speed_int.get_cruising_interval(si::speed{1.0F});

  CHECK(std::get<0>(too_low_interval) == si::length{-1.0F});
  CHECK(std::get<1>(too_low_interval) == si::length{-1.0F});

  // TEST: CRUISING CANDIDATE HAS VALID INTERVAL: INTERSECTION POINT
  auto valid_interval = speed_int.get_cruising_interval(si::speed{2.0F});

  CHECK(std::get<0>(valid_interval) == si::length{3.0F});
  CHECK(std::get<1>(valid_interval) == si::length{9.0F});
}

TEST_CASE("get_cruising_interval::braking_isp") {  // NOLINT
  // ISP - INTERSECTION POINT
  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);

  auto acc = general_driving(si::length{0.0F}, si::length{5.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto braking = general_driving(si::length{5.0F}, si::length{10.0F},
                                 si::speed{5.0F}, si::time{5.0F}, MAX_BRAKING);

  runtime_results rr_acc, rr_braking;

  rr_acc.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{2.0F}, si::length{2.0F}, si::speed{1.5F});
  rr_acc.emplace_back(si::time{3.0F}, si::length{3.0F}, si::speed{2.0F});
  rr_acc.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{2.5F});
  rr_acc.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{3.0F});

  rr_braking.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{3.0F});
  rr_braking.emplace_back(si::time{6.0F}, si::length{6.0F}, si::speed{2.5F});
  rr_braking.emplace_back(si::time{7.0F}, si::length{7.0F}, si::speed{2.0F});
  rr_braking.emplace_back(si::time{8.0F}, si::length{8.0F}, si::speed{1.5F});
  rr_braking.emplace_back(si::time{9.0F}, si::length{9.0F}, si::speed{1.0F});
  rr_braking.emplace_back(si::time{10.0F}, si::length{10.0F}, si::speed{0.5F});

  // LAST RUN ALLOCATION
  acc.last_run_ = rr_acc;
  braking.last_run_ = rr_braking;

  // ASSIGN DRIVING REGIMES
  speed_int.driving_regimes_ = {acc, braking};

  // TEST: CRUISING CANDIDATE HAS VALID INTERVAL: INTERSECTION POINT
  auto valid_interval = speed_int.get_cruising_interval(si::speed{2.0F});

  CHECK(std::get<0>(valid_interval) == si::length{3.0F});
  CHECK(std::get<1>(valid_interval) == si::length{7.0F});
}

TEST_CASE("get_cruising_interval::first_and_last_isp_test") {
  // ISP - INTERSECTION POINT
  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);

  auto acc = general_driving(si::length{0.0F}, si::length{4.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto braking = general_driving(si::length{4.0F}, si::length{8.0F},
                                 si::speed{5.0F}, si::time{5.0F}, MAX_BRAKING);

  runtime_results rr_acc, rr_braking;

  rr_acc.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{2.0F}, si::length{2.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{3.0F}, si::length{3.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{1.5F});

  rr_braking.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{1.5F});
  rr_braking.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{1.0F});
  rr_braking.emplace_back(si::time{6.0F}, si::length{6.0F}, si::speed{1.0F});
  rr_braking.emplace_back(si::time{7.0F}, si::length{7.0F}, si::speed{0.5F});
  rr_braking.emplace_back(si::time{8.0F}, si::length{8.0F}, si::speed{0.5F});

  // LAST RUN ALLOCATION
  acc.last_run_ = rr_acc;
  braking.last_run_ = rr_braking;

  // ASSIGN DRIVING REGIMES
  speed_int.driving_regimes_ = {acc, braking};

  // TEST: CRUISING CANDIDATE HAS VALID INTERVAL: INTERSECTION POINT
  // TESTS: FIRST ISP IN ACCELERATION && LAST ISP IN BRAKING/COASTING
  auto valid_interval = speed_int.get_cruising_interval(si::speed{1.0F});

  CHECK(std::get<0>(valid_interval) == si::length{2.0F});
  CHECK(std::get<1>(valid_interval) == si::length{6.0F});
}

TEST_CASE("get_time_at_position::pos_not_in_speed_interval") {
  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);

  CHECK_THROWS(speed_int.get_time_at_position(si::length{11.0F}));
}

TEST_CASE("get_time_at_position::pos_not_found") {
  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);

  auto shall_position = si::time{-1.0F};
  auto is_position = speed_int.get_time_at_position(si::length{2.0F});

  CHECK(is_position == shall_position);
}

TEST_CASE("get_time_at_position::combined_valid_test_cases") {
  interval_list const ints = {};
  auto speed_int = speed_interval(si::length{0.0F}, si::length{10.0F},
                                  si::speed{5.0F}, ints);

  auto acc = general_driving(si::length{0.0F}, si::length{4.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto cruising = general_driving(si::length{4.0F}, si::length{8.0F},
                                  si::speed{2.5F}, si::time{4.0F}, CRUISING);
  auto braking = general_driving(si::length{8.0F}, si::length{10.0F},
                                 si::speed{2.5F}, si::time{8.0F}, MAX_BRAKING);

  runtime_results rr_acc, rr_cruising, rr_braking;

  rr_acc.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{2.0F}, si::length{2.0F}, si::speed{1.5F});
  rr_acc.emplace_back(si::time{3.0F}, si::length{3.0F}, si::speed{2.0F});
  rr_acc.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{2.5F});

  rr_cruising.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{2.5F});
  rr_cruising.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{2.5F});
  rr_cruising.emplace_back(si::time{6.0F}, si::length{6.0F}, si::speed{2.5F});
  rr_cruising.emplace_back(si::time{7.0F}, si::length{7.0F}, si::speed{2.5F});
  rr_cruising.emplace_back(si::time{8.0F}, si::length{8.0F}, si::speed{2.5F});

  rr_braking.emplace_back(si::time{8.0F}, si::length{8.0F}, si::speed{2.5F});
  rr_braking.emplace_back(si::time{9.0F}, si::length{9.0F}, si::speed{1.25F});
  rr_braking.emplace_back(si::time{10.0F}, si::length{10.0F}, si::speed{0.0F});

  // LAST RUN ALLOCATION
  acc.last_run_ = rr_acc;
  cruising.last_run_ = rr_cruising;
  braking.last_run_ = rr_braking;

  // ASSIGN DRIVING REGIMES
  speed_int.driving_regimes_ = {acc, cruising, braking};

  // TEST POSITION IN FIRST REGIME
  CHECK(speed_int.get_time_at_position(si::length{0.0F}) == si::time{0.0F});
  CHECK(speed_int.get_time_at_position(si::length{2.0F}) == si::time{2.0F});
  CHECK(speed_int.get_time_at_position(si::length{4.0F}) == si::time{4.0F});

  // TEST POSITION IN SECOND AND THIRD REGIME
  CHECK(speed_int.get_time_at_position(si::length{6.0F}) == si::time{6.0F});
  CHECK(speed_int.get_time_at_position(si::length{8.0F}) == si::time{8.0F});

  CHECK(speed_int.get_time_at_position(si::length{9.0F}) == si::time{9.0F});
  CHECK(speed_int.get_time_at_position(si::length{10.0F}) == si::time{10.0F});
}

TEST_CASE("get_transit_time_difference_cruising::base_case_tests") {
  interval_list const ints = {};
  auto speed_int =
      speed_interval(si::length{0.0F}, si::length{8.0F}, si::speed{5.0F}, ints);

  auto acc = general_driving(si::length{0.0F}, si::length{4.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto braking = general_driving(si::length{4.0F}, si::length{8.0F},
                                 si::speed{2.0F}, si::time{4.0F}, MAX_BRAKING);

  runtime_results rr_acc, rr_braking;

  rr_acc.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.0F});
  rr_acc.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{2.0F}, si::length{2.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{3.0F}, si::length{3.0F}, si::speed{1.5F});
  rr_acc.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{2.0F});

  rr_braking.emplace_back(si::time{4.0F}, si::length{4.0F}, si::speed{2.0F});
  rr_braking.emplace_back(si::time{5.0F}, si::length{5.0F}, si::speed{1.5F});
  rr_braking.emplace_back(si::time{6.0F}, si::length{6.0F}, si::speed{1.0F});
  rr_braking.emplace_back(si::time{7.0F}, si::length{7.0F}, si::speed{0.5F});
  rr_braking.emplace_back(si::time{8.0F}, si::length{8.0F}, si::speed{0.0F});

  // LAST RUN ALLOCATION
  acc.last_run_ = rr_acc;
  braking.last_run_ = rr_braking;

  // ASSIGN DRIVING REGIMES
  speed_int.driving_regimes_ = {acc, braking};

  // TEST: NO CHANGE - Cruising Speed too high
  CHECK(speed_int.get_transit_time_difference_cruising(si::speed{2.5F}) ==
        si::time{0.0F});

  // TEST: NO CHANGE - Cruising Speed at max
  CHECK(speed_int.get_transit_time_difference_cruising(si::speed{2.0F}) ==
        si::time{0.0F});
}

TEST_CASE("get_transit_time_difference_cruising::change_in_speed_test") {
  interval_list const ints = {};
  auto speed_int =
      speed_interval(si::length{0.0F}, si::length{8.0F}, si::speed{5.0F}, ints);

  auto acc = general_driving(si::length{0.0F}, si::length{4.0F},
                             si::speed{0.0F}, si::time{0.0F}, MAX_ACCELERATION);
  auto braking = general_driving(si::length{4.0F}, si::length{8.0F},
                                 si::speed{2.0F}, si::time{2.0F}, MAX_BRAKING);

  runtime_results rr_acc, rr_braking;

  rr_acc.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.0F});
  rr_acc.emplace_back(si::time{0.8F}, si::length{1.0F}, si::speed{0.5F});
  rr_acc.emplace_back(si::time{1.4F}, si::length{2.0F}, si::speed{1.0F});
  rr_acc.emplace_back(si::time{1.8F}, si::length{3.0F}, si::speed{1.5F});
  rr_acc.emplace_back(si::time{2.0F}, si::length{4.0F}, si::speed{2.0F});

  rr_braking.emplace_back(si::time{2.0F}, si::length{4.0F}, si::speed{2.0F});
  rr_braking.emplace_back(si::time{2.2F}, si::length{5.0F}, si::speed{1.5F});
  rr_braking.emplace_back(si::time{2.6F}, si::length{6.0F}, si::speed{1.0F});
  rr_braking.emplace_back(si::time{3.2F}, si::length{7.0F}, si::speed{0.5F});
  rr_braking.emplace_back(si::time{4.0F}, si::length{8.0F}, si::speed{0.0F});

  // LAST RUN ALLOCATION
  acc.last_run_ = rr_acc;
  braking.last_run_ = rr_braking;

  // ASSIGN DRIVING REGIMES
  speed_int.driving_regimes_ = {acc, braking};

  // TEST: NO CHANGE - Cruising Speed too high; BASE CASE
  CHECK(speed_int.get_transit_time_difference_cruising(si::speed{2.5F}) ==
        si::time{0.0F});

  // TEST: NO CHANGE - Cruising Speed at max; BASE CASE
  CHECK(speed_int.get_transit_time_difference_cruising(si::speed{2.0F}) ==
        si::time{0.0F});

  // TEST: CHANGE - Cruising Speed at center
  // Calculations:
  //  PRE   [0m, 2m]: transit time = 1.4s - 0.0s = 1.4s
  //  POST  [6m, 8m]: transit time = 4.0s - 2.6s = 1.4s
  //  CRUIS [2m, 6m] = 4m at 1m/s: transit time = 4s
  //  NEW TRANSIT TIME = 1.4s + 4s + 1.4s = 6.8s
  //  CURRENT TRANSIT TIME = 4s
  //  DIFFERENCE = 2.8s
  CHECK(speed_int.get_transit_time_difference_cruising(si::speed{1.0F}) ==
        si::time{2.8F});
}

TEST_SUITE_END();  // NOLINT

TEST_SUITE_BEGIN("sheepmaker");  // NOLINT

// SHEEPMAKER - Initialization
TEST_CASE("sheepmaker_initialzation::double") {  // NOLINT
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{5.0F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  CHECK(data.rr_.begin()->speed_ == si::speed{0.0F});
  CHECK(data.rr_.back().speed_ == si::speed{0.0F});

  CHECK(data.init_driving_regimes_.size() == 2);
  CHECK(data.init_driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.init_driving_regimes_[1].type_ == MAX_BRAKING);
}

TEST_CASE("sheepmaker_initialization::halt_at_end") {  // NOLINT
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  CHECK(data.rr_.begin()->speed_ == si::speed{0.0F});
  CHECK(data.rr_.back().speed_ == si::speed{0.0F});
  CHECK(data.rr_[15].speed_ == si::speed{2.5F});

  CHECK(data.init_driving_regimes_.size() == 4);
  CHECK(data.init_driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.init_driving_regimes_[1].type_ == MAX_BRAKING);
  CHECK(data.init_driving_regimes_[2].type_ == MAX_ACCELERATION);
  CHECK(data.init_driving_regimes_[3].type_ == MAX_BRAKING);
}

TEST_CASE(  // NOLINT
    "sheepmaker::coasting_candidates::interval_has_no_cruising") {
  // REMARK: this is a highly constructed, therefore unrealistic, test case
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  CHECK(data.speed_intervals_[0].driving_regimes_.size() == 2);
  CHECK(data.speed_intervals_[0].driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.speed_intervals_[0].driving_regimes_[1].type_ == MAX_BRAKING);

  auto cp_cand = coasting_candidates(data, data.speed_intervals_[0]);

  CHECK(data.speed_intervals_[0].driving_regimes_.size() == 3);
  CHECK(data.speed_intervals_[0].driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.speed_intervals_[0].driving_regimes_[1].type_ == COASTING);
  CHECK(data.speed_intervals_[0].driving_regimes_[2].type_ == MAX_BRAKING);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{15.0F});

  CHECK(cp_cand.coasting_point_idx_ == 11);
  CHECK(cp_cand.step_size_ == 2);
}

TEST_CASE("sheepmaker::coasting_candidates::interval_has_cruising") {  // NOLINT
  // REMARK: this is a highly constructed, therefore unrealistic, test case
  // ACC: [0, 6], CRUISE: [6, 14], DECELERATE: [14, 15]
  // coasting_point should be at 7, when having frictionless train and a
  // cruising speed of 2.75 meters per second
  // expected result should look like this:
  // ACC: [0, 6], CRUISE: [6, 7], COASTING [7, 14] DECELERATE: [14, 15]
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  // update sheepmaker_data container
  auto cruising = general_driving(si::length{6.0F}, si::length{14.0F},
                                  si::speed{2.75}, si::time{0.0F}, CRUISING);

  data.speed_intervals_[0].driving_regimes_.insert(
      data.speed_intervals_[0].driving_regimes_.end() - 1, cruising);
  data.speed_intervals_[0].driving_regimes_[0].end_ = si::length{6.0F};
  data.speed_intervals_[0].driving_regimes_[2].start_ = si::length{14.0F};

  data.rr_ = run_complete(data.tp_, data.speed_intervals_, data.step_size_);

  CHECK(data.speed_intervals_[0].driving_regimes_.size() == 3);
  CHECK(data.speed_intervals_[0].driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.speed_intervals_[0].driving_regimes_[1].type_ == CRUISING);
  CHECK(data.speed_intervals_[0].driving_regimes_[2].type_ == MAX_BRAKING);

  auto cp_cand = coasting_candidates(data, data.speed_intervals_[0]);

  CHECK(data.speed_intervals_[0].driving_regimes_.size() == 4);
  CHECK(data.speed_intervals_[0].driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.speed_intervals_[0].driving_regimes_[1].type_ == CRUISING);
  CHECK(data.speed_intervals_[0].driving_regimes_[2].type_ == COASTING);
  CHECK(data.speed_intervals_[0].driving_regimes_[3].type_ == MAX_BRAKING);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{6.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{6.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[3].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[3].end_ == si::length{15.0F});

  CHECK(cp_cand.coasting_point_idx_ == 11);
  CHECK(cp_cand.step_size_ == 2);
}

TEST_CASE(  // NOLINT
    "sheepmaker::coasting_candidates::second::interval_has_no_cruising") {
  // REMARK: this is a highly constructed, therefore unrealistic, test case
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  CHECK(data.speed_intervals_[1].driving_regimes_.size() == 2);
  CHECK(data.speed_intervals_[1].driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.speed_intervals_[1].driving_regimes_[1].type_ == MAX_BRAKING);

  auto cp_cand = coasting_candidates(data, data.speed_intervals_[1]);

  CHECK(data.speed_intervals_[1].driving_regimes_.size() == 3);
  CHECK(data.speed_intervals_[1].driving_regimes_[0].type_ == MAX_ACCELERATION);
  CHECK(data.speed_intervals_[1].driving_regimes_[1].type_ == COASTING);
  CHECK(data.speed_intervals_[1].driving_regimes_[2].type_ == MAX_BRAKING);

  // NOTE: speed limit is 2.5 m/s, starting velocity is 2.5 m/s

  CHECK(data.speed_intervals_[1].driving_regimes_[0].start_ ==
        si::length{15.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[0].end_ == si::length{20.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].start_ ==
        si::length{20.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].end_ == si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[2].start_ ==
        si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[2].end_ == si::length{25.0F});

  CHECK(cp_cand.coasting_point_idx_ == 20);
  CHECK(cp_cand.step_size_ == 2);
}

TEST_CASE(  // NOLINT
    "sheepmaker::coasting_candidates::set_next_cp_cand::no_cruising::right") {
  // REMARK: this is a highly constructed, therefore unrealistic, test case
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  auto cp_cand = coasting_candidates(data, data.speed_intervals_[0]);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{15.0F});

  CHECK(cp_cand.coasting_point_idx_ == 11);
  CHECK(cp_cand.step_size_ == 2);

  cp_cand.set_next_cp_candidate(data, data.speed_intervals_[0], si::time{5.0F},
                                si::time{0.0F});

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{13.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{13.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{15.0F});

  CHECK(cp_cand.coasting_point_idx_ == 13);
  CHECK(cp_cand.step_size_ == 1);
}

TEST_CASE(  // NOLINT
    "sheepmaker::coasting_candidates::set_next_cp_cand::no_cruising::left") {
  // REMARK: this is a highly constructed, therefore unrealistic, test case
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  auto cp_cand = coasting_candidates(data, data.speed_intervals_[0]);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{15.0F});

  CHECK(cp_cand.coasting_point_idx_ == 11);
  CHECK(cp_cand.step_size_ == 2);

  cp_cand.set_next_cp_candidate(data, data.speed_intervals_[0], si::time{5.0F},
                                si::time{10.0F});

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{15.0F});

  CHECK(cp_cand.coasting_point_idx_ == 11);
  CHECK(cp_cand.step_size_ == 1);
}

TEST_CASE(  // NOLINT
    "sheepmaker::update_coasting_point::no_cruising::always_right") {
  // REMARK: this is a highly constructed, therefore unrealistic, test case
  // First: add coasting point in first speed interval
  // propagate coasting point to the right to get faster and faster, since the
  // target time is 0 s
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp = generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{15.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[0].start_ ==
        si::length{15.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[0].end_ == si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].start_ ==
        si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].end_ == si::length{25.0F});

  sheepmaker_update_coasting_point(data, data.speed_intervals_[0], 0.00F);
  sheepmaker_update_coasting_point(data, data.speed_intervals_[1], 0.1F);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{13.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{13.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{15.0F});

  // friction less -> coasting is like cruising -> no change in speed
  // no change in speed -> no change in travel time -> set coasting point
  // (needed) and stop

  CHECK(data.speed_intervals_[1].driving_regimes_[0].start_ ==
        si::length{15.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[0].end_ == si::length{20.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].start_ ==
        si::length{20.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].end_ == si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[2].start_ ==
        si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[2].end_ == si::length{25.0F});
}

TEST_CASE("sheepmaker::sheepmaker_per_halt" * doctest::skip(true)) {  // NOLINT
  // REMARK: this is a highly constructed, therefore unrealistic, test case
  interval_list const input = {
      interval(si::length{0.0F}, si::speed{5.0F}, true, type::HALT),
      interval(si::length{10.0F}, si::speed{5.0F}),
      interval(si::length{15.0F}, si::speed{2.5F}),
      interval(si::length{25.0F}, si::speed{2.5F}, true, type::HALT)};

  auto const tp =
      generate_frictionless_test_train_2();  // generate_frictionless_test_train_2();
  auto data = sheepmaker_data(tp, input, {1.0F});

  sheepmaker_initialization(data);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{15.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[0].start_ ==
        si::length{15.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[0].end_ == si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].start_ ==
        si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].end_ == si::length{25.0F});

  CHECK(!data.speed_intervals_[0].halt_);
  CHECK(data.speed_intervals_[1].halt_);

  sheepmaker_per_halt(data);

  CHECK(data.speed_intervals_[0].driving_regimes_[0].start_ ==
        si::length{0.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[0].end_ == si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].start_ ==
        si::length{11.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[1].end_ == si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].start_ ==
        si::length{14.0F});
  CHECK(data.speed_intervals_[0].driving_regimes_[2].end_ == si::length{15.0F});

  // frictionless -> coasting is like cruising -> no change in speed
  // no change in speed -> no change in travel time -> set coasting point
  // (needed) and stop

  CHECK(data.speed_intervals_[1].driving_regimes_[0].start_ ==
        si::length{15.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[0].end_ == si::length{20.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].start_ ==
        si::length{20.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[1].end_ == si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[2].start_ ==
        si::length{22.0F});
  CHECK(data.speed_intervals_[1].driving_regimes_[2].end_ == si::length{25.0F});
}

TEST_CASE("sheepmaker::complete" * doctest::skip(true)) {  // NOLINT
  infrastructure const infra(SMALL_OPTS);
  timetable const tt(FOLLOW_OPTS, infra);

  auto train = tt->trains_.front();

  type_set const record_events({type::HALT});
  type_set const border_types({type::SPEED_LIMIT, type::HALT,
                               type::APPROACH_SIGNAL, type::MAIN_SIGNAL});

  auto const intervals =
      get_interval_list(*train, record_events, border_types, infra);

  auto const tp = generate_test_train_2();

  auto start = high_resolution_clock::now();
  auto halt_to_halt_containers =
      sheepmaker(tp, intervals, *train, si::length{1.0F});
  auto stop = high_resolution_clock::now();

  auto duration = duration_cast<milliseconds>(stop - start);

  std::cout << duration.count() << std::endl;
  /**
   * This test is a sheepmaker eetc demonstration.
   *
   * This demonstration consists of three halt-to-halt sections (3 halts, after
   * initial halt, constant speed limit)
   *
   *
   * Driving-Regimes Differences Time and Energy Efficient (sheepmaker v0.1)
   * -----------------------------------------------------------------------
   *
   * halt-to-halt 1st: 0m - 3157m
   * TYPE   || ACCELERATION ||   COASTING   || DECELERATION
   * TE     ||  0m - 2676m  ||   --------   || 2676m - 3157m
   * EE     ||  0m - 900m   || 900m - 3157m || 3157m - 3157m
   *
   * halt-to-halt 2nd: 3157m - 6072m
   * TYPE   ||  ACCELERATION  ||    COASTING   || DECELERATION
   * TE     || 3157m - 5652m  ||   ----------  || 5652m - 6133m
   * EE     || 3157m - 3880m  || 3880m - 6133m || 6133m - 6133m
   *
   * halt-to-halt 2nd: 6072m - 11323m
   * TYPE   ||  ACCELERATION  ||    COASTING    || DECELERATION
   * TE     || 6133m - 10842m ||   ----------   || 10842m - 11323m
   * EE     || 6133m - 9022m  || 9022m - 11323m || 11323m - 11323m
   *
   *
   * Time Vs. Energy Efficient Train Driving Times
   * ---------------------------------------------
   *
   * (halt0 - halt1) || target time     || time efficient   || sheepmaker v0.1
   * (0 - 3157)      || 300 s           || 96.34 s          || 227.69 s
   * (3157 - 6133)   || 600 s           || 92.34 s          || 222.26 s
   * (6133 - 11323)  || 600 s           || 141.32 s         || 414.74 s
   *
   */

  si::time const target_time_1{
      static_cast<float>(halt_to_halt_containers[0].end_.arrival_ -
                         halt_to_halt_containers[0].start_.departure_)};
  si::time const target_time_2{
      static_cast<float>(halt_to_halt_containers[1].end_.arrival_ -
                         halt_to_halt_containers[1].start_.departure_)};
  si::time const target_time_3{
      static_cast<float>(halt_to_halt_containers[2].end_.arrival_ -
                         halt_to_halt_containers[2].start_.departure_)};

  std::cout << "H2H #1 TARGET TIME: " << target_time_1 << std::endl;
  std::cout << "H2H #1 EE TIME: " << halt_to_halt_containers[0].rr_.back().time_
            << std::endl;
  std::cout << "H2H #2 TARGET TIME: " << target_time_2 << std::endl;
  std::cout << "H2H #2 EE TIME: " << halt_to_halt_containers[1].rr_.back().time_
            << std::endl;
  std::cout << "H2H #3 TARGET TIME: " << target_time_3 << std::endl;
  std::cout << "H2H #3 EE TIME: " << halt_to_halt_containers[2].rr_.back().time_
            << std::endl;
}

TEST_SUITE_END();  // NOLINT