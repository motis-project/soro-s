#include "doctest/doctest.h"

#include "soro/utls/parse_fp.h"

#include "soro/infrastructure/route.h"
#include "soro/rolling_stock/ctc.h"
#include "soro/rolling_stock/freight.h"

using namespace soro::rs;
using namespace soro::utls;
using namespace soro::infra;

TEST_SUITE("static asserts") {

  TEST_CASE("skip_omitted") {
    static_assert(!static_cast<bool>(skip_omitted::OFF));
    static_assert(static_cast<bool>(skip_omitted::ON));
    static_assert(static_cast<skip_omitted>(false) == skip_omitted::OFF);
    static_assert(static_cast<skip_omitted>(true) == skip_omitted::ON);
  }

  TEST_CASE("ctc") {
    static_assert(!static_cast<bool>(CTC::NO));
    static_assert(static_cast<bool>(CTC::YES));
    static_assert(static_cast<CTC>(false) == CTC::NO);
    static_assert(static_cast<CTC>(true) == CTC::YES);
  }

  TEST_CASE("freight") {
    static_assert(!static_cast<bool>(FreightTrain::NO));
    static_assert(static_cast<bool>(FreightTrain::YES));
    static_assert(static_cast<FreightTrain>(false) == FreightTrain::NO);
    static_assert(static_cast<FreightTrain>(true) == FreightTrain::YES);
  }

  TEST_CASE("rising") {
    static_assert(!static_cast<bool>(rising::NO));
    static_assert(static_cast<bool>(rising::YES));
    static_assert(static_cast<rising>(false) == rising::NO);
    static_assert(static_cast<rising>(true) == rising::YES);
  }

  TEST_CASE("replace_comma") {
    static_assert(!static_cast<bool>(replace_comma::OFF));
    static_assert(static_cast<bool>(replace_comma::ON));
    static_assert(static_cast<replace_comma>(false) == replace_comma::OFF);
    static_assert(static_cast<replace_comma>(true) == replace_comma::ON);
  }
}
