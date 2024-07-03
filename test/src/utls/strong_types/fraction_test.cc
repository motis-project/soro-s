#include "doctest/doctest.h"

#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "soro/base/fp_precision.h"

#include "soro/utls/strong_types/fraction.h"
#include "soro/utls/strong_types/type_list.h"

#include "soro/si/units.h"

using namespace soro;
using namespace soro::si;
using namespace soro::utls;

TEST_CASE("min") {  // NOLINT
  static_assert(1 == min_v<std::integral_constant<size_t, 1>,
                           std::integral_constant<size_t, 2>>);

  static_assert(1 == min_v<std::integral_constant<size_t, 2>,
                           std::integral_constant<size_t, 1>>);

  static_assert(3 == min_v<std::integral_constant<size_t, 11>,
                           std::integral_constant<size_t, 3>>);

  static_assert(8 == min_v<std::integral_constant<size_t, 8>,
                           std::integral_constant<size_t, 11>>);
}

TEST_CASE("fraction simplify") {  // NOLINT
  auto one_square = from_m(1) * from_m(1);
  static_assert(one_square.is<area>());

  auto one_m = one_square / from_m(1);
  static_assert(std::is_same_v<decltype(one_m), length>);
  static_assert(one_m.is<length>());

  auto one_s = soro::si::time{1};
  auto speed_one = one_m / one_s;
  static_assert(speed_one.is<speed>());

  auto acceleration_one = speed_one / one_s;
  static_assert(acceleration_one.is<accel>());

  auto acceleration_two = acceleration_one * 2.5;
  static_assert(acceleration_two.is<accel>());
  CHECK_LE(acceleration_two, accel{2.5});
  CHECK_GE(acceleration_two, accel{2.5});

  auto speed_two = acceleration_two * soro::si::time{2};
  static_assert(speed_two.is<speed>());
  CHECK_LE(speed_two, speed{5});
  CHECK_GE(speed_two, speed{5});
}

TEST_CASE("fraction simplify2 test") {  // NOLINT
  using frac_type_1 =
      fraction<type_list<second_tag>, type_list<meter_tag>, double>;
  using frac_type_2 = accel;

  using result_type = fraction<type_list<>, type_list<second_tag>, double>;

  static_assert(std::is_same_v<decltype(std::declval<frac_type_1>() *
                                        std::declval<frac_type_2>()),
                               result_type>);

  static_assert(is_same_type_list_v<type_list<meter_tag, second_tag>,
                                    type_list<second_tag, meter_tag>>);

  static_assert(
      is_same_fraction_v<type_list<meter_tag, second_tag>, type_list<>, double,
                         type_list<second_tag, meter_tag>, type_list<>,
                         double>);
}

TEST_CASE("fraction pow") {  // NOLINT
  si::speed const one{2};

  auto speed_squared = one.template pow<2>();
  static_assert(
      std::is_same_v<decltype(speed_squared),
                     decltype(std::declval<speed>() * std::declval<speed>())>);

  auto speed_cubed = one.template pow<3>();
  static_assert(
      std::is_same_v<decltype(speed_cubed),
                     decltype(speed_squared * std::declval<speed>())>);

  CHECK(equal(speed_squared.val_, std::pow(one.val_, 2)));
  CHECK(equal(speed_cubed.val_, std::pow(one.val_, 3)));
}

TEST_CASE("fraction sqrt") {
  si::speed const two{2};
  auto const four = two * two;
  auto const sixteen = four * four;

  auto const two_sqrt = four.sqrt();
  auto const four_sqrt = sixteen.sqrt();

  static_assert(std::is_same_v<decltype(two_sqrt), si::speed const>);
  static_assert(std::is_same_v<decltype(four_sqrt), decltype(four)>);

  CHECK_EQ(two_sqrt, two);
  CHECK_EQ(four_sqrt, four);
}

TEST_CASE("fraction sqrt mixed") {
  si::speed const two{2};
  auto const four = two * two;
  auto const sixteen = four * four;

  si::weight const one{1};
  auto const one_squared = one * one;

  auto const mixed = sixteen * one_squared;

  auto const two_sqrt = four.sqrt();
  auto const four_sqrt = sixteen.sqrt();

  auto const mixed_sqrt = mixed.sqrt();

  static_assert(std::is_same_v<decltype(two_sqrt), si::speed const>);
  static_assert(std::is_same_v<decltype(four_sqrt), decltype(four)>);
  static_assert(
      std::is_same_v<decltype(mixed_sqrt), const decltype(four_sqrt * one)>);

  CHECK_EQ(two_sqrt, two);
  CHECK_EQ(four_sqrt, four);
  CHECK_EQ(mixed_sqrt, four * one);
}

TEST_CASE("is_fraction") {  // NOLINT
  static_assert(is_fraction_v<fraction<type_list<>, type_list<>, double>>);
  static_assert(!is_fraction_v<double>);
}