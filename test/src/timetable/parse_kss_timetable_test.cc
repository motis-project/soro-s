#include "doctest/doctest.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

TEST_SUITE("KSS") {
  TEST_CASE("parse simple kss") {}
}

TEST_SUITE("parse de_kss" *
           doctest::skip(!std::filesystem::exists(DE_KSS_FOLDER))) {

  TEST_CASE("parse de_kss") {
    auto opts = DE_ISS_OPTS;
    opts.determine_interlocking_ = true;
    infrastructure const infra(opts);
    //    infra.save("infra.raw");
    timetable const tt(DE_KSS_OPTS, infra);
    //    std::cout << "Got " << tt->train_store_.size() << " trains" <<
    //    std::endl; tt.save("tt.raw");
  }
}
