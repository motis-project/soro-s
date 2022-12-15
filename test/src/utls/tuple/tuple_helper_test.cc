#include "doctest/doctest.h"

#include "soro/base/soro_types.h"
#include "soro/utls/tuple/apply_at.h"
#include "soro/utls/tuple/find_if.h"
#include "soro/utls/tuple/for_each.h"
#include "soro/utls/tuple/is_tuple.h"

using namespace soro;
using namespace soro::utls;
using namespace soro::utls::tuple;

TEST_CASE("is_tuple test") {  // NOLINT
  soro::tuple<int, int, int> const t = {1, 2, 3};
  static_assert(is_tuple_v<decltype(t)>);
  static_assert(!is_tuple_v<std::size_t>);
}

TEST_CASE("find_if test") {  // NOLINT
  soro::tuple<int, int, int> const t = {1, 2, 3};
  CHECK(find_if(t, [](auto const& e) { return e == 2; }) == 1);
}

TEST_CASE("apply_at test") {  // NOLINT
  soro::tuple<int, int, int> t = {1, 2, 3};
  apply_at(t, 1, [](auto& e) { e += 5; });
  CHECK(get<1>(t) == 7);

  soro::tuple<int, int, int> const t_const = {1, 2, 3};
  int e_1 = 0;
  apply_at(t_const, 1, [&](auto const& e) { e_1 = e; });
  CHECK(e_1 == 2);
}

TEST_CASE("for_each test") {  // NOLINT
  soro::tuple<int, int, int> t = {1, 2, 3};
  for_each(t, [](auto& e) { e += e; });
  CHECK(get<0>(t) == 2);
  CHECK(get<1>(t) == 4);
  CHECK(get<2>(t) == 6);

  soro::tuple<int, int, int> const t_const = {1, 2, 3};
  int acc = 0;
  for_each(t_const, [&](auto const& e) { acc += e; });
  CHECK(acc == 6);
}
