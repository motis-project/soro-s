#include "doctest/doctest.h"

#include "soro/rolling_stock/train_series.h"
#include "soro/si/units.h"
#include "soro/utls/math/piecewise_function.h"

#if defined(SERIALIZE)
#include "cista/serialization.h"
#endif

using namespace soro;
using namespace soro::rs;
using namespace soro::utls;

TEST_CASE("tractive force piecewise test") {  // NOLINT
  auto pf1 = make_polynomial(tractive_force_3_t{10.0}, tractive_force_2_t{0.1},
                             tractive_force_1_t{0.5});
  auto pf2 = make_polynomial(tractive_force_3_t{11.0}, tractive_force_2_t{0.2},
                             tractive_force_1_t{0.8});

  si::speed const ten{10.0};
  si::speed const hundred{100.0};

  auto result = pf1(ten);

  CHECK(result == si::force{1001.5});
  static_assert(std::is_same_v<decltype(result), si::force>);

  auto tfc = make_piecewise(make_piece(pf1, si::speed{0.0}, si::speed{60.0}),
                            make_piece(pf2, si::speed{60.0}, si::speed{160.0}));

  auto result2 = pf2(hundred);
  CHECK(result2 == si::force{110020.8});

  CHECK(result == tfc(ten));
  CHECK(result2 == tfc(hundred));
}

#if !(defined(__EMSCRIPTEN__) || defined(SORO_SAN))
TEST_CASE("piecewise jump should throw") {  // NOLINT
  auto const p1 = make_polynomial(0.2, 0.3, 0.4);
  auto const p2 = make_polynomial(1.2, 2.3, 3.4);

  CHECK_THROWS(
      make_piecewise(make_piece(p1, 0.0, 10.0), make_piece(p2, 15.0, 20.0)));
}

TEST_CASE("piecewise out of bounds should throw") {  // NOLINT
  auto const p1 = make_polynomial(0.2, 0.3, 0.4);
  auto const p2 = make_polynomial(1.2, 2.3, 3.4);

  auto const pf =
      make_piecewise(make_piece(p1, 0.0, 10.0), make_piece(p2, 10.0, 20.0));

  CHECK(pf(20.0 - 0.000001));
  CHECK_THROWS(pf(20.0));
  CHECK_THROWS(pf(20.001));
}
#endif

#if defined(SERIALIZE)
TEST_CASE("serialize piecewise") {  // NOLINT

  std::vector<unsigned char> buf;
  {
    auto const p1 = make_polynomial(0.2, 0.3, 0.4);
    auto const p2 = make_polynomial(1.2, 2.3, 3.4);

    auto const pf =
        make_piecewise(make_piece(p1, 0.0, 10.0), make_piece(p2, 10.0, 20.0));

    buf = cista::serialize(pf);
  }

  auto deserialized_pf = cista::deserialize<
      piecewise_function<piece<polynomial<double, double, double>, double>>>(
      buf);

  CHECK(equal(deserialized_pf->operator()(0.5), 0.6));
  CHECK(equal(deserialized_pf->operator()(10.5), 159.85));
}
#endif
