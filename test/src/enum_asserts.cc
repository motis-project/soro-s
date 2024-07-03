#include "doctest/doctest.h"

#include <cstdint>

#include "soro/utls/parse_fp.h"
#include "soro/utls/std_wrapper/all_of.h"

#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"

#include "soro/rolling_stock/stop_mode.h"

using namespace soro::rs;
using namespace soro::utls;
using namespace soro::infra;

TEST_SUITE("static asserts") {
  TEST_CASE("stop_mode") {
    static_assert(!static_cast<bool>(stop_mode::passenger));
    static_assert(static_cast<bool>(stop_mode::freight));
    static_assert(static_cast<stop_mode>(false) == stop_mode::passenger);
    static_assert(static_cast<stop_mode>(true) == stop_mode::freight);
  }

  TEST_CASE("mileage_dir") {
    static_assert(!static_cast<bool>(mileage_dir::falling));
    static_assert(static_cast<bool>(mileage_dir::rising));
    static_assert(static_cast<mileage_dir>(false) == mileage_dir::falling);
    static_assert(static_cast<mileage_dir>(true) == mileage_dir::rising);
  }

  TEST_CASE("replace_comma") {
    static_assert(!static_cast<bool>(replace_comma::OFF));
    static_assert(static_cast<bool>(replace_comma::ON));
    static_assert(static_cast<replace_comma>(false) == replace_comma::OFF);
    static_assert(static_cast<replace_comma>(true) == replace_comma::ON);
  }

  TEST_CASE("skip") {
    static_assert(!static_cast<bool>(skip::No));
    static_assert(static_cast<bool>(skip::Yes));
    static_assert(static_cast<skip>(false) == skip::No);
    static_assert(static_cast<skip>(true) == skip::Yes);
  }

  constexpr bool true_exactly_once(type const t) {
    auto const trues = static_cast<uint32_t>(is_end_element(t)) +
                       static_cast<uint32_t>(is_simple_element(t)) +
                       static_cast<uint32_t>(is_simple_switch(t)) +
                       static_cast<uint32_t>(is_cross(t)) +
                       static_cast<uint32_t>(is_track_element(t));

    return trues == 1;
  }

  TEST_CASE("check type enum") {
    // every type should only belong to one of the following categories:
    // end, simple, switch, cross, directed or undirected track element
    static_assert(all_of(all_types(), true_exactly_once));
  }
}
