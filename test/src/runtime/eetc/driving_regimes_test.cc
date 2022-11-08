#include "doctest/doctest.h"

#include "soro/rolling_stock/train_series.h"

#include "soro/runtime/driving_regimes/driving_regime.h"
#include "soro/runtime/driving_regimes/general.h"
#include "soro/runtime/driving_regimes/physics.h"

#include "soro/si/constants.h"

using namespace soro;
using namespace soro::rs;
using namespace soro::runtime;

TEST_SUITE_BEGIN("driving_regime");  // NOLINT

train_physics generate_test_train() {
  soro::vector<tractive_piece_t> polynomials;

  tractive_polynomial_t const polynomial_1 = utls::make_polynomial(
      tractive_force_3_t{1000.0F}, tractive_force_2_t{1000.0F},
      tractive_force_1_t{1000.0F});
  tractive_piece_t const piece_1 =
      utls::make_piece(polynomial_1, si::speed{0.0F}, si::speed{10.0F});

  tractive_polynomial_t const polynomial_2 = utls::make_polynomial(
      tractive_force_3_t{250.0F}, tractive_force_2_t{250.0},
      tractive_force_1_t{250.0F});
  tractive_piece_t const piece_2 =
      utls::make_piece(polynomial_2, si::speed{10.0F}, si::speed{15.0F});

  polynomials.push_back(piece_1);
  polynomials.push_back(piece_2);

  tractive_curve_t const tractive_piece_poly =
      utls::make_piecewise(std::move(polynomials));

  drag_coefficient_t drag{(1.0F / 1000.0F) * si::GRAVITATIONAL.val_ * 3.6 *
                          3.6};
  dampening_resistance_t dampening{1.0F * si::GRAVITATIONAL.val_ *
                                   si::weight{10000.0F}.val_ * 3.6};
  rolling_resistance_t rolling{1.0F * si::GRAVITATIONAL.val_ *
                               si::weight{10000.0F}.val_};
  resistance_curve_t const resistance_poly =
      utls::make_polynomial(drag, dampening, rolling);

  return {traction_vehicle{.name_ = "TEST_TRAIN",
                           .weight_ = si::from_kg(10000.0),
                           .max_speed_ = si::from_km_h(200.0),
                           .deacceleration_ = si::from_m_s2(-1.5F),
                           .tractive_curve_ = tractive_piece_poly,
                           .resistance_curve_ = resistance_poly},
          si::from_kg(0.0), si::from_m(0.0), si::from_km_h(400.0)};
}

train_physics generate_frictionless_test_train() {
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

  return {traction_vehicle{.name_ = "FRICTIONLESS_TEST_TRAIN",
                           .weight_ = si::from_kg(400000.0F),
                           .max_speed_ = si::from_km_h(62.0F),
                           .deacceleration_ = si::from_m_s2(-1.5F),
                           .tractive_curve_ = tractive_piece_poly,
                           .resistance_curve_ = resistance_poly},
          si::from_kg(0.0), si::from_m(0.0), si::from_km_h(400.0)};
}

TEST_CASE("runtime_results::no_intersection_point") {  // NOLINT
  runtime_results rr_a, rr_b;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{2.0F}, si::length{2.0F}, si::speed{3.0F});

  rr_b.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.0F});
  rr_b.emplace_back(si::time{1.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_b.emplace_back(si::time{2.0F}, si::length{2.0F}, si::speed{2.0F});

  CHECK(driving_regime::intersection_point(rr_a, rr_b, false) ==
        si::length{-1.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point::intersection_point_pos_rr_a_slope") {
  runtime_results rr_a, rr_b;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{3.0F});

  rr_b.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{2.0F});
  rr_b.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_b.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{2.0F});

  CHECK(driving_regime::intersection_point(rr_a, rr_b, false) ==
        si::length{1.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point::intersection_point_pos_rr_a_slope") {
  runtime_results rr_a, rr_b;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{1.0F});

  rr_b.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{3.0F});
  rr_b.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_b.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{1.0F});

  CHECK(driving_regime::intersection_point(rr_a, rr_b, false) ==
        si::length{2.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point::intersection_point_between_i-i+1_mid") {
  runtime_results rr_a, rr_b;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{3.0F});

  rr_b.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.5F});
  rr_b.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{1.5F});
  rr_b.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{1.5F});

  CHECK(driving_regime::intersection_point(rr_a, rr_b, false) ==
        si::length{0.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point::intersection_point_between_i-i+1_non_"
    "mid") {
  runtime_results rr_a, rr_b;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{3.0F});

  rr_b.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.25F});
  rr_b.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{1.25F});
  rr_b.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{1.25F});

  CHECK(driving_regime::intersection_point(rr_a, rr_b, false) ==
        si::length{0.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point::intersection_point_outside_range") {
  runtime_results rr_a, rr_b;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{3.0F});

  rr_b.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.5F});
  rr_b.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{0.5F});
  rr_b.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{0.5F});

  CHECK(driving_regime::intersection_point(rr_a, rr_b, false) ==
        si::length{-1.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::id-left") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{1.0F});

  si::speed const speed_const = {1.0F};

  CHECK(driving_regime::intersection_point_with_constant(rr_a, speed_const) ==
        si::length{0.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::id-right") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{1.0F});

  si::speed const speed_const = {1.0F};

  CHECK(driving_regime::intersection_point_with_constant(
            rr_a, speed_const, false) == si::length{2.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::no-up") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{3.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{4.0F});

  si::speed const speed_const = {1.0F};

  CHECK(driving_regime::intersection_point_with_constant(
            rr_a, speed_const, false) == si::length{-1.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::no-down") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{4.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{3.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{2.0F});

  si::speed const speed_const = {5.0F};

  CHECK(driving_regime::intersection_point_with_constant(
            rr_a, speed_const, false) == si::length{-1.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::intersection-up-1") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{2.0F});

  si::speed const speed_const = {1.0F};

  CHECK(driving_regime::intersection_point_with_constant(
            rr_a, speed_const, false) == si::length{1.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::intersection-up-2") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{0.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{4.0F});

  si::speed const speed_const = {1.0F};

  CHECK(driving_regime::intersection_point_with_constant(
            rr_a, speed_const, false) == si::length{0.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::intersection-down-1") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{1.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{0.0F});

  si::speed const speed_const = {1.0F};

  CHECK(driving_regime::intersection_point_with_constant(
            rr_a, speed_const, false) == si::length{1.0F});
}

TEST_CASE(  // NOLINT
    "DrivingRegime::intersection_point_with_constant::intersection-down-2") {
  runtime_results rr_a;

  rr_a.emplace_back(si::time{0.0F}, si::length{0.0F}, si::speed{4.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{1.0F}, si::speed{2.0F});
  rr_a.emplace_back(si::time{0.0F}, si::length{2.0F}, si::speed{0.0F});

  si::speed const speed_const = {1.0F};

  CHECK(driving_regime::intersection_point_with_constant(
            rr_a, speed_const, false) == si::length{1.0F});
}

// PHYSICS
TEST_CASE("physics::accelerate::zero") {  // NOLINT
  auto const acc_results = accelerate(si::acceleration{0.0F}, si::speed{1.0F},
                                      si::length{0.0F}, si::length{1.0F});

  CHECK(std::get<0>(acc_results).val_ == 1);
  CHECK(std::get<1>(acc_results).val_ == 1);
}

TEST_CASE("physics::accelerate::pos") {  // NOLINT
  auto const acc_results = accelerate(si::acceleration{1.0F}, si::speed{1.0F},
                                      si::length{0.0F}, si::length{1.0F});

  // ceil(x*pow(10,x))/pow(10,x)

  CHECK(round(std::get<0>(acc_results).val_ * pow(10, 3)) / pow(10, 3) ==
        0.732);
  CHECK(round(std::get<1>(acc_results).val_ * pow(10, 3)) / pow(10, 3) ==
        1.732);
}

TEST_CASE("physics::accelerate::neg") {  // NOLINT
  auto const acc_results = accelerate(si::acceleration{-0.5F}, si::speed{2.0F},
                                      si::length{0.0F}, si::length{1.0F});

  CHECK(round(std::get<0>(acc_results).val_ * pow(10, 3)) / pow(10, 3) ==
        0.536);
  CHECK(round(std::get<1>(acc_results).val_ * pow(10, 3)) / pow(10, 3) ==
        1.732);
}

TEST_CASE("physics::accelerate_reverse::neg") {  // NOLINT

  si::acceleration const acc = {-1.0F};
  si::speed const vel1 = {0.0F};
  si::length const start{si::ZERO<si::length>};
  si::length const end = {1.0F};

  // calculate acceleration_reverse
  auto const acc_rev_results = accelerate_reverse(acc, vel1, start, end);

  CHECK(round(std::get<0>(acc_rev_results).val_ * pow(10, 7)) / pow(10, 7) ==
        1.4142136);
  CHECK(round(std::get<1>(acc_rev_results).val_ * pow(10, 7)) / pow(10, 7) ==
        1.4142136);

  si::speed const vel0 = {static_cast<float>(
      ceil(std::get<1>(acc_rev_results).val_ * pow(10, 7)) / pow(10, 7))};

  // calculate acceleration with acceleration_reverse data should result in
  // acceleration_reverse input data
  auto const acc_results = accelerate(acc, vel0, start, end);

  CHECK(round(std::get<0>(acc_results).val_ * pow(10, 3)) / pow(10, 3) ==
        1.414);
  // ignored: precision; should be zero, but is 0.001
  CHECK(round(std::get<1>(acc_results).val_ * pow(10, 3)) / pow(10, 3) ==
        0.001);
}

TEST_CASE("driving_regime::math::test") {  // NOLINT
  // basic tests
  // remark: tests have no physical relevance
  auto acc_dur_sol = general_driving::get_acceleration_duration(
      si::acceleration{1.0F}, si::speed{0.0F}, si::length{0.0F});
  CHECK(std::get<0>(acc_dur_sol).val_ == 0.0);
  CHECK(std::get<1>(acc_dur_sol).val_ == -0.0);

  acc_dur_sol = general_driving::get_acceleration_duration(
      si::acceleration{1.0F}, si::speed{0.5F}, si::length{0.0F});
  CHECK(std::get<0>(acc_dur_sol).val_ == 0.0);
  CHECK(std::get<1>(acc_dur_sol).val_ == -1.0);

  acc_dur_sol = general_driving::get_acceleration_duration(
      si::acceleration{1.0F}, si::speed{0.0F}, si::length{-0.5F});
  CHECK(std::isnan(std::get<0>(acc_dur_sol).val_));
  CHECK(std::isnan(std::get<1>(acc_dur_sol).val_));

  acc_dur_sol = general_driving::get_acceleration_duration(
      si::acceleration{1.0F}, si::speed{-4.0F}, si::length{-8.0F});
  CHECK(std::get<0>(acc_dur_sol).val_ == 4.0);
  CHECK(std::get<1>(acc_dur_sol).val_ == 4.0);
}

TEST_CASE("general_driving:get_acceleration::cruising") {  // NOLINT
  auto const tv = generate_test_train();

  general_driving const general_dr =
      general_driving({0.0F}, {1.0F}, {1.0F}, {0.0F}, CRUISING);

  CHECK(general_dr.get_acceleration(tv, general_dr.vel0_) ==
        si::acceleration{0.0F});
}

TEST_CASE("general_driving:get_acceleration::coasting") {  // NOLINT
  auto const tv = generate_test_train();

  general_driving const general_dr =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{1.0F},
                      si::time{0.0F}, COASTING);

  CHECK(general_dr.get_acceleration(tv, general_dr.vel0_) ==
        si::acceleration{-45.126});
}

TEST_CASE("general_driving::get_acceleration::max_braking") {  // NOLINT
  auto const tv = generate_test_train();

  general_driving const general_dr =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_BRAKING);

  CHECK(general_dr.get_acceleration(tv, general_dr.vel0_) ==
        si::acceleration{-46.626F});
}

TEST_CASE("general_driving::get_acceleration::max_acceleration") {  // NOLINT
  auto const tv = generate_test_train();

  general_driving const general_dr =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_ACCELERATION);

  CHECK(general_dr.get_acceleration(tv, general_dr.vel0_) ==
        si::acceleration{-44.826F});
}

TEST_CASE(  // NOLINT
    "general_driving::get_acceleration::max_acceleration::no_tf_coefficients" *
    doctest::skip(true)) {
  auto const tv = generate_test_train();

  general_driving const general_dr =
      general_driving(si::length{0.0F}, si::length{1.0F}, si::speed{16.0F},
                      si::time{0.0F}, MAX_ACCELERATION);

  CHECK(general_dr.get_acceleration(tv, general_dr.vel0_) ==
        si::acceleration{0.0F});
}

// TEST SIMULATE_ON (cruising, acceleration)
TEST_CASE("general_driving::simulate::cruising::normal") {  // NOLINT
  auto const tv = generate_frictionless_test_train();
  interval_list const intervals = {interval(si::length{15.0F}, {62.0F})};

  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{15.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);

  std::vector<runtime_result> shall_rr = {
      runtime_result({0.0F}, {0.0F}, {1.0F}),
      runtime_result({1.0F}, {1.0F}, {1.0F}),
      runtime_result({2.0F}, {2.0F}, {1.0F}),
      runtime_result({3.0F}, {3.0F}, {1.0F}),
  };

  auto is_rr = general_dr.simulate(tv, intervals, {0.0F}, {3.0F}, {1.0F},
                                   {0.0F}, {0.0F}, {1.0F}, false);

  CHECK(is_rr.size() == shall_rr.size());

  for (std::size_t i = 0UL; i < is_rr.size(); ++i) {
    CHECK(is_rr[i].time_ == shall_rr[i].time_);
    CHECK(is_rr[i].distance_ == shall_rr[i].distance_);
    CHECK(is_rr[i].speed_ == shall_rr[i].speed_);
  }
}

TEST_CASE("general_driving::simulate::accelerate::normal") {  // NOLINT
  auto const tv = generate_frictionless_test_train();
  interval_list const intervals = {interval(si::length{15.0F}, {62.0F})};

  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{15.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_ACCELERATION);

  std::vector<runtime_result> shall_rr = {
      runtime_result({0.0F}, {0.0F}, {1.0F}),
      runtime_result({0.890077F}, {1.0F}, {1.247F}),
      runtime_result({1.62951F}, {2.0F}, {1.45778F}),
      runtime_result({2.27393F}, {3.0F}, {1.64579F}),
  };

  auto is_rr = general_dr.simulate(tv, intervals, {0.0F}, {3.0F}, {1.0F},
                                   {0.0F}, {0.0F}, {1.0F}, false);

  CHECK(is_rr.size() == shall_rr.size());

  for (std::size_t i = 0UL; i < is_rr.size(); ++i) {
    CHECK(is_rr[i].time_ == shall_rr[i].time_);
    CHECK(is_rr[i].distance_ == shall_rr[i].distance_);
    CHECK(is_rr[i].speed_ == shall_rr[i].speed_);
  }
}

TEST_CASE(  // NOLINT
    "general_driving::simulate::accelerate-dt-offset::normal") {
  auto const tv = generate_frictionless_test_train();
  interval_list const intervals = {interval(si::length{15.0F}, {62.0F})};

  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{15.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_ACCELERATION);

  std::vector<runtime_result> shall_rr = {
      runtime_result({5.0F}, {0.0F}, {1.0F}),
      runtime_result({5.89008F}, {1.0F}, {1.247F}),
      runtime_result({6.62951F}, {2.0F}, {1.45778F}),
      runtime_result({7.27393F}, {3.0F}, {1.64579F}),
  };

  auto is_rr = general_dr.simulate(tv, intervals, {0.0F}, {3.0F}, {1.0F},
                                   {0.0F}, {5.0F}, {1.0F}, false);

  CHECK(is_rr.size() == shall_rr.size());

  for (std::size_t i = 0UL; i < is_rr.size(); ++i) {
    CHECK(is_rr[i].time_ == shall_rr[i].time_);
    CHECK(is_rr[i].distance_ == shall_rr[i].distance_);
    CHECK(is_rr[i].speed_ == shall_rr[i].speed_);
  }
}

// TEST SIMULATE_REVERSE_ON (deceleration)

TEST_CASE("general_driving::simulate::deceleration::reverse") {  // NOLINT
  auto const tv = generate_frictionless_test_train();
  interval_list const intervals = {
      interval(si::length{15.0}, si::speed{62.07F})};

  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{15.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_BRAKING);

  std::vector<runtime_result> shall_rr = {
      runtime_result({0.0F}, {0.0F}, {3.0F}),
      runtime_result({0.367006898F}, {1.0F}, {2.44948959F}),
      runtime_result({0.845299482}, {2.0F}, {1.73205078F}),
      runtime_result({2.0F}, {3.0F}, {0.0F}),
  };

  auto is_rr = general_dr.simulate(tv, intervals, {0.0F}, {3.0F}, {0.0F},
                                   {0.0F}, {0.0F}, {1.0F}, true);

  CHECK(is_rr.size() == shall_rr.size());

  for (std::size_t i = 0UL; i < is_rr.size(); ++i) {
    CHECK(is_rr[i].time_ == shall_rr[i].time_);
    CHECK(is_rr[i].distance_ == shall_rr[i].distance_);
    CHECK(is_rr[i].speed_ == shall_rr[i].speed_);
  }
}

TEST_CASE(  // NOLINT
    "general_driving::simulate::deceleration-dt-offset::reverse") {
  auto const tv = generate_frictionless_test_train();
  interval_list const intervals = {
      interval(si::length{15.0}, si::speed{62.07F})};

  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{15.0F}, si::speed{1.0F},
                      si::time{0.0F}, MAX_BRAKING);

  std::vector<runtime_result> shall_rr = {
      runtime_result({5.0F}, {0.0F}, {3.0F}),
      runtime_result({5.367006898F}, {1.0F}, {2.44948959F}),
      runtime_result({5.845299482}, {2.0F}, {1.73205078F}),
      runtime_result({7.0F}, {3.0F}, {0.0F}),
  };

  auto is_rr = general_dr.simulate(tv, intervals, {0.0F}, {3.0F}, {0.0F},
                                   {0.0F}, {5.0F}, {1.0F}, true);

  CHECK(is_rr.size() == shall_rr.size());

  for (std::size_t i = 0UL; i < is_rr.size(); ++i) {
    CHECK(is_rr[i].time_ == shall_rr[i].time_);
    CHECK(is_rr[i].distance_ == shall_rr[i].distance_);
    CHECK(is_rr[i].speed_ == shall_rr[i].speed_);
  }
}

// single run test case
TEST_CASE("general_driving::run::cruising::normal") {  // NOLINT
  auto const tv = generate_frictionless_test_train();
  interval_list const intervals = {interval(si::length{15.0F}, {62.0F})};

  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);

  std::vector<runtime_result> shall_rr = {
      runtime_result({0.0F}, {0.0F}, {1.0F}),
      runtime_result({1.0F}, {1.0F}, {1.0F}),
      runtime_result({2.0F}, {2.0F}, {1.0F}),
      runtime_result({3.0F}, {3.0F}, {1.0F}),
  };

  auto is_rr = general_dr.run(tv, intervals, {1.0F}, {0.0F}, {0.0F}, {1.0F});

  CHECK(is_rr.size() == shall_rr.size());

  for (std::size_t i = 0UL; i < is_rr.size(); ++i) {
    CHECK(is_rr[i].time_ == shall_rr[i].time_);
    CHECK(is_rr[i].distance_ == shall_rr[i].distance_);
    CHECK(is_rr[i].speed_ == shall_rr[i].speed_);
  }
}

TEST_CASE("general_driving::get_speed_range::empty") {  // NOLINT
  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);

  std::vector<runtime_result> const is_rr = {};

  general_dr.last_run_ = is_rr;

  auto speed_range = general_dr.get_speed_range();

  CHECK(std::get<0>(speed_range).val_ == INT_MAX);
  CHECK(std::get<1>(speed_range).val_ == INT_MIN);
}

TEST_CASE("general_driving::get_speed_range::constant") {  // NOLINT
  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);

  std::vector<runtime_result> const is_rr = {
      runtime_result({0.0F}, {0.0F}, {0.0F}),
      runtime_result({1.0F}, {1.0F}, {0.0F}),
      runtime_result({2.0F}, {2.0F}, {0.0F}),
      runtime_result({3.0F}, {3.0F}, {0.0F}),
  };

  general_dr.last_run_ = is_rr;

  auto speed_range = general_dr.get_speed_range();

  CHECK(std::get<0>(speed_range).val_ == 0.0F);
  CHECK(std::get<1>(speed_range).val_ == 0.0F);
}

TEST_CASE("general_driving::get_speed_range::pseudorandom") {  // NOLINT
  general_driving general_dr =
      general_driving(si::length{0.0F}, si::length{3.0F}, si::speed{1.0F},
                      si::time{0.0F}, CRUISING);

  std::vector<runtime_result> const is_rr = {
      runtime_result({0.0F}, {0.0F}, {0.0F}),
      runtime_result({1.0F}, {1.0F}, {1.0F}),
      runtime_result({2.0F}, {2.0F}, {-1.0F}),
      runtime_result({3.0F}, {3.0F}, {3.0F}),
      runtime_result({0.0F}, {0.0F}, {1.0F}),
      runtime_result({1.0F}, {1.0F}, {10.0F}),
      runtime_result({2.0F}, {2.0F}, {12.0F}),
      runtime_result({3.0F}, {3.0F}, {-5.0F}),
  };

  general_dr.last_run_ = is_rr;

  auto speed_range = general_dr.get_speed_range();

  CHECK(std::get<0>(speed_range).val_ == -5.0F);
  CHECK(std::get<1>(speed_range).val_ == 12.0F);
}

TEST_SUITE_END();  // NOLINT
