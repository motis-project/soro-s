#include "doctest/doctest.h"

#include <cstdint>
#include <type_traits>

#include "soro/utls/strong_types/type_list.h"

using namespace soro;
using namespace soro::utls;

TEST_CASE("count") {  // NOLINT
  static_assert(count_v<int, type_list<>> == 0);
  static_assert(count_v<int, type_list<int>> == 1);
  static_assert(count_v<int, type_list<float>> == 0);
  static_assert(count_v<int, type_list<float, int, int>> == 2);
  static_assert(count_v<int, type_list<float, uint32_t, int>> == 1);
  static_assert(count_v<int, type_list<float, int32_t, int>> == 2);
  static_assert(
      count_v<double, type_list<double, int, double, float, double>> == 3);
}

TEST_CASE("remove once") {  // NOLINT
  static_assert(std::is_same_v<type_list<>, remove_once_t<int, type_list<>>>);
  static_assert(
      std::is_same_v<type_list<float>, remove_once_t<int, type_list<float>>>);
  static_assert(
      std::is_same_v<type_list<>, remove_once_t<int, type_list<int>>>);
  static_assert(
      std::is_same_v<type_list<int>, remove_once_t<int, type_list<int, int>>>);
  static_assert(
      std::is_same_v<type_list<float, int, double>,
                     remove_once_t<int, type_list<int, float, int, double>>>);
  static_assert(std::is_same_v<
                type_list<int, int, double, int>,
                remove_once_t<int, type_list<int, int, int, double, int>>>);
}

TEST_CASE("push front") {  // NOLINT
  static_assert(
      std::is_same_v<push_front_t<int, type_list<int>>, type_list<int, int>>);
  static_assert(std::is_same_v<push_front_t<int, type_list<float>>,
                               type_list<int, float>>);
  static_assert(std::is_same_v<push_front_t<int, type_list<float, float>>,
                               type_list<int, float, float>>);
  static_assert(std::is_same_v<push_front_t<void, type_list<float, float>>,
                               type_list<float, float>>);
}

TEST_CASE("remove times") {  // NOLINT
  static_assert(std::is_same_v<type_list<float>,
                               remove_times_t<2, int, type_list<int, float>>>);
  static_assert(
      std::is_same_v<type_list<int, int>,
                     remove_times_t<2, int, type_list<int, int, int, int>>>);
  static_assert(
      std::is_same_v<
          type_list<double, int>,
          remove_times_t<2, double, type_list<double, double, double, int>>>);
  static_assert(
      std::is_same_v<
          type_list<int, int, int>,
          remove_times_t<2, double, type_list<double, double, int, int, int>>>);
  static_assert(std::is_same_v<
                type_list<double, int, int>,
                remove_times_t<2, int, type_list<double, int, int, int, int>>>);
  static_assert(std::is_same_v<
                type_list<int, double, int>,
                remove_times_t<2, int, type_list<int, int, int, double, int>>>);
}

TEST_CASE("concat") {  // NOLINT
  static_assert(std::is_same_v<type_list<int, float>,
                               concat_t<type_list<int>, type_list<float>>>);
  static_assert(
      std::is_same_v<type_list<int, float, int, double>,
                     concat_t<type_list<int, float>, type_list<int, double>>>);
  static_assert(
      std::is_same_v<type_list<>, concat_t<type_list<>, type_list<>>>);
}

TEST_CASE("concat_times") {  // NOLINT
  static_assert(
      std::is_same_v<type_list<int>, concat_times_t<1, type_list<int>>>);

  static_assert(std::is_same_v<type_list<int, int>,
                               concat_times<2, type_list<int>>::type>);

  static_assert(!std::is_same_v<type_list<int, int>,
                                concat_times<3, type_list<int>>::type>);

  static_assert(std::is_same_v<type_list<int, int, int>,
                               concat_times<3, type_list<int>>::type>);

  static_assert(std::is_same_v<type_list<float, int, float, int>,
                               concat_times<2, type_list<float, int>>::type>);
}