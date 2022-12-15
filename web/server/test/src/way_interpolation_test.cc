#include "doctest/doctest.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/utls/coordinates/cartesian.h"
#include "soro/utls/coordinates/gps.h"

#include "soro/server/osm_export/interpolation.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::si;
using namespace soro::utls;
using namespace soro::infra;
using namespace soro::server::osm_export;

TEST_SUITE("interpolation") {

  TEST_CASE("interpolation") {
    infrastructure const infra(soro::test::SMALL_OPTS);

    for (auto const& station : infra->stations_) {
      for (const border& border : station->borders_) {
        auto const interpolation =
            compute_interpolation(border.element_, border.neighbour_element_,
                                  infra->element_positions_);
        CHECK_EQ(interpolation.first_elem_, border.element_->id());
        CHECK_EQ(interpolation.second_elem_, border.neighbour_element_->id());
        for (auto bezier_elem : interpolation.points_) {
          CHECK_EQ(typeid(bezier_elem).name(), typeid(gps).name());
        }
      }
    }
  }
}
