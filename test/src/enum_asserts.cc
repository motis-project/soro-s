#include "doctest/doctest.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

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

  constexpr bool true_exactly_once(type const t) {
    auto const trues = static_cast<uint32_t>(is_end_element(t)) +
                       static_cast<uint32_t>(is_simple_element(t)) +
                       static_cast<uint32_t>(is_simple_switch(t)) +
                       static_cast<uint32_t>(is_cross(t)) +
                       static_cast<uint32_t>(is_directed_track_element(t)) +
                       static_cast<uint32_t>(is_undirected_track_element(t));

    return trues == 1;
  }

  TEST_CASE("check type enum") {
    // every type should only belong to one of the following categories:
    // end, simple, switch, cross, directed or undirected track element
    static_assert(all_of(all_types(), true_exactly_once));
  }
}
