#include "doctest/doctest.h"

#include "utl/to_set.h"

#include "soro/utls/unixtime.h"

#include "soro/base/fp_precision.h"
#include "soro/simulation/dpd.h"

using namespace soro;
using namespace soro::utls;
using namespace soro::simulation;

TEST_CASE("round to nearest multiple unixtime") {  // NOLINT
  std::vector<unixtime> nums = {unixtime(0), unixtime(1), unixtime(2),
                                unixtime(3), unixtime(4), unixtime(5),
                                unixtime(6), unixtime(7)};

  auto const multiple = default_granularity::get<unixtime>();
  std::vector<unixtime> results = {unixtime(0), unixtime(0), unixtime(0),
                                   unixtime(0), unixtime(6), unixtime(6),
                                   unixtime(6), unixtime(6)};

  CHECK(nums.size() == results.size());

  for (auto i = 0U; i < nums.size(); ++i) {
    auto const result = round_to_nearest_multiple(nums[i], multiple);
    CHECK(result == results[i]);
  }
}

TEST_CASE("round to nearest multiple kilometer_per_hor") {  // NOLINT
  std::vector<kilometer_per_hour> nums = {
      kilometer_per_hour{1},  kilometer_per_hour{2}, kilometer_per_hour{3},
      kilometer_per_hour{4},  kilometer_per_hour{5}, kilometer_per_hour{6},
      kilometer_per_hour{7},  kilometer_per_hour{8}, kilometer_per_hour{9},
      kilometer_per_hour{10}, kilometer_per_hour{11}};
  auto const multiple = default_granularity::get<kilometer_per_hour>();
  std::vector<kilometer_per_hour> results = nums;

  CHECK(nums.size() == results.size());

  for (auto i = 0U; i < nums.size(); ++i) {
    auto const result = round_to_nearest_multiple(nums[i], multiple);
    CHECK(result == results[i]);
  }
}

TEST_CASE("dpd simple") {  // NOLINT
  using DPD = dpd<default_granularity, unixtime, kilometer_per_hour>;
  DPD dpd;

  std::vector<unixtime> times = {
      unixtime(0), unixtime(12), unixtime(2), unixtime(3), unixtime(4),
      unixtime(5), unixtime(16), unixtime(8), unixtime(9), unixtime(10)};

  std::vector<kilometer_per_hour> speeds = {
      kilometer_per_hour{0}, kilometer_per_hour{1}, kilometer_per_hour{2},
      kilometer_per_hour{3}, kilometer_per_hour{4}, kilometer_per_hour{6},
      kilometer_per_hour{7}, kilometer_per_hour{8}, kilometer_per_hour{9},
      kilometer_per_hour{10}};

  for (auto idx = 0U; idx < times.size(); ++idx) {
    dpd.insert(times[idx], speeds[idx], 0.1F);
  }

  auto const granular_times = utl::to_set(times, [&](unixtime const& t) {
    return round_to_nearest_multiple(t, DPD::get_granularity<unixtime>());
  });

  auto const granular_speeds =
      utl::to_set(speeds, [&](kilometer_per_hour const& kmh) {
        return round_to_nearest_multiple(
            kmh, DPD::get_granularity<kilometer_per_hour>());
      });

  std::size_t found_entries = 0;
  for (auto const& [time, speed_dpd] : dpd) {
    for (auto const [speed, prob] : speed_dpd) {
      if (prob <= probability_t{0.0}) {
        continue;
      }

      ++found_entries;

      auto const found_time = granular_times.contains(time);
      auto const found_speed = granular_speeds.contains(speed);

      CHECK(found_time);
      CHECK(found_speed);
      CHECK(equal(prob, probability_t(0.1)));
    }
  }

  CHECK(found_entries == times.size());
}

TEST_CASE("dpd sums to one") {  // NOLINT
  dpd<default_granularity, unixtime, kilometer_per_hour> dpd;

  std::vector<unixtime> times = {unixtime(0),  unixtime(12), unixtime(2),
                                 unixtime(3),  unixtime(5),  unixtime(6),
                                 unixtime(16), unixtime(9),  unixtime(10)};

  std::vector<kilometer_per_hour> speeds = {
      kilometer_per_hour{0}, kilometer_per_hour{1}, kilometer_per_hour{2},
      kilometer_per_hour{3}, kilometer_per_hour{4}, kilometer_per_hour{6},
      kilometer_per_hour{7}, kilometer_per_hour{8}, kilometer_per_hour{9},
      kilometer_per_hour{10}};

  for (auto idx = 0U; idx < times.size(); ++idx) {
    dpd.insert(times[idx], speeds[idx], 0.1F);
  }

  CHECK(std::abs(sum(dpd) - 0.9F) <= 0.001F);
}

TEST_CASE("dpd iterate") {
  dpd<default_granularity, unixtime, kilometer_per_hour> dpd;

  std::vector<unixtime> times = {unixtime(0),  unixtime(12), unixtime(2),
                                 unixtime(3),  unixtime(5),  unixtime(6),
                                 unixtime(16), unixtime(9),  unixtime(10)};

  std::vector<kilometer_per_hour> speeds = {
      kilometer_per_hour{0}, kilometer_per_hour{1}, kilometer_per_hour{2},
      kilometer_per_hour{3}, kilometer_per_hour{4}, kilometer_per_hour{6},
      kilometer_per_hour{7}, kilometer_per_hour{8}, kilometer_per_hour{9},
      kilometer_per_hour{10}};

  for (auto idx = 0U; idx < times.size(); ++idx) {
    dpd.insert(times[idx], speeds[idx], 0.1F);
  }

  std::size_t counter = 0;

  std::vector<unixtime> found_times;
  std::vector<kilometer_per_hour> found_speeds;
  std::vector<probability_t> found_probabilities;

  for (auto const& [time, speed, prob] : iterate(dpd)) {
    ++counter;

    found_times.emplace_back(time);
    found_speeds.emplace_back(speed);
    found_probabilities.emplace_back(prob);
  }

  std::vector<unixtime> expected_times = {
      unixtime{0}, unixtime{0},  unixtime{0},  unixtime{6},  unixtime{6},
      unixtime{6}, unixtime{12}, unixtime{12}, unixtime{18},
  };

  CHECK(found_times == expected_times);

  CHECK(counter == 9);
}