#include "doctest/doctest.h"

#include <iostream>

#include "soro/dpd.h"
#include "soro/granularity.h";

using namespace soro;

TEST_CASE("one_dim_dpd_test") {
  dpd<TimeGranularity, unixtime> time_dpd{};
      }
