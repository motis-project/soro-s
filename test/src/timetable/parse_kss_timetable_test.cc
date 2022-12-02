#include "doctest/doctest.h"

#include "soro/utls/algo/overlap.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/exclusion.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

TEST_SUITE("parse de_kss") {
  TEST_CASE("parse de_kss") {
    auto opts = soro::test::DE_ISS_OPTS;
    infrastructure const infra(opts);
    timetable const tt(soro::test::DE_KSS_OPTS, infra);
  }
}
