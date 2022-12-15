#include "doctest/doctest.h"

#include <string>

#include "soro/utls/tuple/slice.h"

using namespace soro::utls::tuple;
using namespace soro::utls::tuple::detail;

TEST_SUITE("tuple slice") {

  TEST_CASE("make integer sequence") {
    static_assert(
        std::is_same_v<decltype(make_integer_sequence<std::size_t, 0, 1>()),
                       std::integer_sequence<std::size_t, 0>>);

    static_assert(
        std::is_same_v<decltype(make_integer_sequence<std::size_t, 0, 2>()),
                       std::integer_sequence<std::size_t, 0, 1>>);

    static_assert(
        std::is_same_v<decltype(make_integer_sequence<std::size_t, 0, 3>()),
                       std::integer_sequence<std::size_t, 0, 1, 2>>);

    static_assert(
        std::is_same_v<decltype(make_integer_sequence<std::size_t, 0, 100>()),
                       std::make_index_sequence<100>>);

    static_assert(
        std::is_same_v<decltype(make_integer_sequence<std::size_t, 1, 2>()),
                       std::integer_sequence<std::size_t, 1>>);

    static_assert(
        std::is_same_v<decltype(make_integer_sequence<std::size_t, 3, 6>()),
                       std::integer_sequence<std::size_t, 3, 4, 5>>);

    static_assert(
        std::is_same_v<decltype(make_integer_sequence<std::size_t, 3, 3>()),
                       std::integer_sequence<std::size_t>>);
  }

  TEST_CASE("slice") {
    using t1 = std::tuple<std::string, float, double>;

    static_assert(std::is_same_v<decltype(slice<0, 0>(std::declval<t1>())),
                                 std::tuple<>>);
    static_assert(std::is_same_v<decltype(slice<0, 1>(std::declval<t1>())),
                                 std::tuple<std::string>>);
    static_assert(std::is_same_v<decltype(slice<0, 2>(std::declval<t1>())),
                                 std::tuple<std::string, float>>);
    static_assert(
        std::is_same_v<decltype(slice<0, 3>(std::declval<t1>())), t1>);
    static_assert(std::is_same_v<decltype(slice<1, 3>(std::declval<t1>())),
                                 std::tuple<float, double>>);
    static_assert(std::is_same_v<decltype(slice<2, 3>(std::declval<t1>())),
                                 std::tuple<double>>);
    static_assert(std::is_same_v<decltype(slice<3, 3>(std::declval<t1>())),
                                 std::tuple<>>);
  }
}
