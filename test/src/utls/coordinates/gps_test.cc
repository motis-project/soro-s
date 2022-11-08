#include "doctest/doctest.h"

#include <vector>

#include "utl/pairwise.h"
#include "utl/zip.h"

#include "soro/utls/coordinates/cartesian.h"
#include "soro/utls/coordinates/gps.h"

#include "soro/base/fp_precision.h"

using namespace soro;
using namespace soro::utls;

TEST_SUITE("interpolation") {

  TEST_CASE("check distance") {
    std::vector<gps> const gps_coords = {
        gps{.lon_ = 13.404954, .lat_ = 52.520007},
        gps{.lon_ = 105.834160, .lat_ = 21.027764},
        gps{.lon_ = 126.977969, .lat_ = 37.566535},
        gps{.lon_ = 8.651193, .lat_ = 49.872825}};

    std::vector<gps::precision> const true_distances = {
        8336.62706396484, 2744.785346869459, 8579.334391059};

    std::vector<gps::precision> results;

    for (auto const [gps1, gps2] : utl::pairwise(gps_coords)) {
      results.push_back(gps1.distance(gps2) / 1000.0);
    }

    for (auto const [expected, got] : utl::zip(true_distances, results)) {
      CHECK(equal(expected, got));
    }
  }

  TEST_CASE("gps cartesian conversion test") {
    std::vector<gps> const gps_coords = {
        gps{.lon_ = 0.0, .lat_ = 0.0},
        gps{.lon_ = 8.42346234, .lat_ = 49.93242346},
        gps{.lon_ = 3.4543290, .lat_ = -30.00454387},
        gps{.lon_ = 1.3143123, .lat_ = -89.32432},
        gps{.lon_ = 150.64926683, .lat_ = 68.8973246},
        gps{.lon_ = 74.005941, .lat_ = 40.712784},
        gps{.lon_ = 13.404954, .lat_ = 52.520007},
        gps{.lon_ = 105.834160, .lat_ = 21.027764},
        gps{.lon_ = 126.977969, .lat_ = 37.566535},
        gps{.lon_ = 8.651193, .lat_ = 49.872825}};

    for (gps const coord_expected : gps_coords) {
      cartesian const cartesian = coord_expected.to_cartesian();
      gps const coord_computed = cartesian.to_gps();

      CHECK(equal(coord_expected.lon_, coord_computed.lon_));
      CHECK(equal(coord_expected.lat_, coord_computed.lat_));
    }
  }
}