#include "doctest/doctest.h"

#include <type_traits>

#include "soro/base/fp_precision.h"

#include "soro/utls/math/polynomial.h"

#if defined(SERIALIZE)
#include "cista/serialization.h"
#endif

namespace soro::test {

using namespace soro::utls;

TEST_CASE("polynomial test") {  // NOLINT
  auto zero_p = polynomial();
  auto res = zero_p(0.5);
  static_assert(std::is_same_v<decltype(res), double>);
  CHECK(equal(zero_p(0.5), 0.0));
  CHECK(equal(zero_p(0.5F), 0.0F));

  auto p0 = make_polynomial(0.2);
  CHECK(equal(p0(0.5), 0.2));
  CHECK(equal(p0(1.5), 0.2));

  auto const p1 = make_polynomial(0.2, 0.3);
  static_assert(std::is_same_v<decltype(p1), const polynomial<double, double>>);
  CHECK(equal(p1(0.5), 0.4));

  auto const p2 = make_polynomial(0.2, 0.3, 0.4);
  static_assert(
      std::is_same_v<decltype(p2), const polynomial<double, double, double>>);
  static_assert(p2.degree() == 2);
  CHECK(equal(p2(0.5), 0.6));
}

#if defined(SERIALIZE)
TEST_CASE("serialize polynomial") {  // NOLINT
  std::vector<unsigned char> buf;
  {
    auto const p = make_polynomial(0.2, 0.3, 0.4);
    buf = cista::serialize(p);
  }

  auto deserialized_p =
      cista::deserialize<polynomial<double, double, double>>(buf);

  CHECK(equal(deserialized_p->operator()(0.5), 0.6));
}
#endif

}  // namespace soro::test
