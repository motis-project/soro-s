#include "doctest/doctest.h"

#include <string>
#include <vector>

#include "date/date.h"

#include "soro/base/time.h"

#include "soro/utls/container/optional.h"
#include "soro/utls/std_wrapper/sort.h"

#include "soro/timetable/bitfield.h"
#include "soro/timetable/interval.h"
#include "soro/timetable/sequence_point.h"
#include "soro/timetable/train.h"

namespace soro::test {

using namespace date;
using namespace soro::tt;

using opt_t = utls::optional<relative_time>;

std::vector<absolute_time> gather_midnights(std::vector<train> const& trains,
                                            interval const& interval) {
  std::vector<absolute_time> result;

  for (auto const& train : trains) {
    for (auto const midnight : train.departures(interval)) {
      result.emplace_back(midnight);
    }
  }

  utls::sort(result);

  return result;
}

train test_train1() {
  date::year_month_day const start_date = 2022_y / March / 1;
  date::year_month_day const end_date = 2022_y / March / 9;

  std::string const bits1 = "111111111";

  sequence_point const dep = {.type_ = sequence_point::type::PASSENGER,
                              .arrival_ = opt_t{hours{7}},
                              .departure_ = opt_t{hours{8}}};
  sequence_point const arr = {.type_ = sequence_point::type::PASSENGER,
                              .arrival_ = opt_t{hours{16}},
                              .departure_ = opt_t{hours{17}}};

  train t1;
  t1.id_ = 1;
  t1.service_days_ = make_bitfield(start_date, end_date, bits1.data());
  t1.sequence_points_.push_back(dep);
  t1.sequence_points_.push_back(arr);

  return t1;
}

train test_train2() {
  date::year_month_day const start_date = 2022_y / March / 1;
  date::year_month_day const end_date = 2022_y / March / 9;

  std::string const bits2 = "101010101";

  sequence_point const first_dep2 = {.type_ = sequence_point::type::PASSENGER,
                                     .arrival_ = opt_t{hours{7}},
                                     .departure_ = opt_t{hours{8}}};
  sequence_point const last_arr2 = {.type_ = sequence_point::type::PASSENGER,
                                    .arrival_ = opt_t{hours{16}},
                                    .departure_ = opt_t{hours{17}}};

  train t2;
  t2.id_ = 2;
  t2.service_days_ = make_bitfield(start_date, end_date, bits2.data());
  t2.sequence_points_.push_back(first_dep2);
  t2.sequence_points_.push_back(last_arr2);

  return t2;
}

train test_train3() {
  date::year_month_day const start_date = 2022_y / March / 1;
  date::year_month_day const end_date = 2022_y / March / 9;

  std::string const bits3 = "000000001";

  sequence_point const first_dep3 = {.type_ = sequence_point::type::PASSENGER,
                                     .arrival_ = opt_t{hours{21}},
                                     .departure_ = opt_t{hours{22}}};
  sequence_point const last_arr3 = {.type_ = sequence_point::type::PASSENGER,
                                    .arrival_ = opt_t{hours{26}},
                                    .departure_ = opt_t{hours{27}}};

  train t3;
  t3.id_ = 3;
  t3.service_days_ = make_bitfield(start_date, end_date, bits3.data());
  t3.sequence_points_.push_back(first_dep3);
  t3.sequence_points_.push_back(last_arr3);

  return t3;
}

train test_train4() {
  date::year_month_day const start_date = 2022_y / March / 1;
  date::year_month_day const end_date = 2022_y / March / 9;

  std::string const bits4 = "100000000";

  sequence_point const first_dep4 = {.type_ = sequence_point::type::PASSENGER,
                                     .arrival_ = opt_t{hours{0}},
                                     .departure_ = opt_t{hours{0}}};
  sequence_point const last_arr4 = {.type_ = sequence_point::type::PASSENGER,
                                    .arrival_ = opt_t{hours{10}},
                                    .departure_ = opt_t{hours{10}}};

  train t4;
  t4.id_ = 4;
  t4.service_days_ = make_bitfield(start_date, end_date, bits4.data());
  t4.sequence_points_.push_back(first_dep4);
  t4.sequence_points_.push_back(last_arr4);

  return t4;
}

train test_train5() {
  date::year_month_day const start_date = 2022_y / March / 1;
  date::year_month_day const end_date = 2022_y / March / 9;

  std::string const bits5 = "010101010";

  sequence_point const first_dep5 = {.type_ = sequence_point::type::PASSENGER,
                                     .arrival_ = opt_t{hours{8}},
                                     .departure_ = opt_t{hours{8}}};
  sequence_point const last_arr5 = {.type_ = sequence_point::type::PASSENGER,
                                    .arrival_ = opt_t{hours{16}},
                                    .departure_ = opt_t{hours{16}}};

  train t5;
  t5.id_ = 5;
  t5.service_days_ = make_bitfield(start_date, end_date, bits5.data());
  t5.sequence_points_.push_back(first_dep5);
  t5.sequence_points_.push_back(last_arr5);

  return t5;
}

TEST_SUITE("iterate train departures 1") {
  TEST_CASE("iterate train departures, all") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 1), ymd_to_abs(2022_y / March / 2),
        ymd_to_abs(2022_y / March / 3), ymd_to_abs(2022_y / March / 4),
        ymd_to_abs(2022_y / March / 5), ymd_to_abs(2022_y / March / 6),
        ymd_to_abs(2022_y / March / 7), ymd_to_abs(2022_y / March / 8),
        ymd_to_abs(2022_y / March / 9),
    };

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval{});

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, none before") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {.start_ = ymd_to_abs(2022_y / February / 10),
                               .end_ = ymd_to_abs(2022_y / February / 12)};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, none after") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 10),
                               .end_ = ymd_to_abs(2022_y / March / 12)};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, earlier half") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 1), ymd_to_abs(2022_y / March / 2),
        ymd_to_abs(2022_y / March / 3)};

    interval const interval = {.start_ = ymd_to_abs(2022_y / February / 15),
                               .end_ = ymd_to_abs(2022_y / March / 4)};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, later half") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 4), ymd_to_abs(2022_y / March / 5),
        ymd_to_abs(2022_y / March / 6), ymd_to_abs(2022_y / March / 7),
        ymd_to_abs(2022_y / March / 8), ymd_to_abs(2022_y / March / 9),
    };

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 4),
                               .end_ = ymd_to_abs(2022_y / March / 14)};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, two hours with train, start") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 6),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 6) + hours{7},
        .end_ = ymd_to_abs(2022_y / March / 6) + hours{9}};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
  TEST_CASE("iterate train departures, two hours with train, middle") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 6),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 6) + hours{10},
        .end_ = ymd_to_abs(2022_y / March / 6) + hours{12}};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, two hours with train, end") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 6),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 6) + hours{15},
        .end_ = ymd_to_abs(2022_y / March / 6) + hours{17}};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, max hours without train") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 6) + hours{16} + seconds{1},
        .end_ = ymd_to_abs(2022_y / March / 7) + hours{8} - seconds{1}};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, min hours with 3 trains") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 6),
        ymd_to_abs(2022_y / March / 7),
        ymd_to_abs(2022_y / March / 8),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 6) + hours{16},
        .end_ = ymd_to_abs(2022_y / March / 8) + hours{8}};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, midnight to midnight") {
    std::vector<train> const trains = {test_train1()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 6)};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 6),
                               .end_ = ymd_to_abs(2022_y / March / 7)};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
}

TEST_SUITE("iterate train departures 2") {
  TEST_CASE("test trains 2, iterate train departures, all") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 1), ymd_to_abs(2022_y / March / 3),
        ymd_to_abs(2022_y / March / 5), ymd_to_abs(2022_y / March / 7),
        ymd_to_abs(2022_y / March / 9)};

    auto const result = gather_midnights(trains, interval{});

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, none") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 10),
                               .end_ = ymd_to_abs(2022_y / March / 12)};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, earlier half") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 1), ymd_to_abs(2022_y / March / 3)};

    interval const interval = {.start_ = ymd_to_abs(2022_y / February / 15),
                               .end_ = ymd_to_abs(2022_y / March / 4)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, later half") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 5), ymd_to_abs(2022_y / March / 7),
        ymd_to_abs(2022_y / March / 9)};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 4),
                               .end_ = ymd_to_abs(2022_y / March / 14)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, two hours with train, start") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 5),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 5) + hours{7},
        .end_ = ymd_to_abs(2022_y / March / 5) + hours{9}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
  TEST_CASE("iterate train departures, two hours with train, middle") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 5),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 5) + hours{10},
        .end_ = ymd_to_abs(2022_y / March / 5) + hours{12}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, two hours with train, end") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 5),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 5) + hours{15},
        .end_ = ymd_to_abs(2022_y / March / 5) + hours{17}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, max hours without train") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 5) + hours{16} + seconds{1},
        .end_ = ymd_to_abs(2022_y / March / 7) + hours{8} - seconds{1}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, min hours with 3 trains") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 5),
        ymd_to_abs(2022_y / March / 7),
        ymd_to_abs(2022_y / March / 9),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 5) + hours{16},
        .end_ = ymd_to_abs(2022_y / March / 9) + hours{8}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("iterate train departures, midnight to midnight") {
    std::vector<train> const trains = {test_train2()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 6),
                               .end_ = ymd_to_abs(2022_y / March / 7)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
}

TEST_SUITE("iterate train departures 3") {
  TEST_CASE("test trains 3, iterate train departures, all") {
    std::vector<train> const trains = {test_train3()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 9)};

    auto const result = gather_midnights(trains, interval{});

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 3, iterate train departures, none") {
    std::vector<train> const trains = {test_train3()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / February / 1),
        .end_ = ymd_to_abs(2022_y / March / 9) + hours{22} - seconds{1}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 3, iterate train departures, one") {
    std::vector<train> const trains = {test_train3()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 9)};

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / February / 1),
        .end_ = ymd_to_abs(2022_y / March / 9) + hours{22}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
}

TEST_SUITE("iterate train departures 4") {
  TEST_CASE("test trains 4, iterate train departures, all") {
    std::vector<train> const trains = {test_train4()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 1)};

    auto const result = gather_midnights(trains, interval{});

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 4, iterate train departures, none") {
    std::vector<train> const trains = {test_train4()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 1) + hours{10} + seconds{1},
        .end_ = ymd_to_abs(2022_y / March / 30)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 4, iterate train departures, one") {
    std::vector<train> const trains = {test_train4()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 1)};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 1),
                               .end_ = ymd_to_abs(2022_y / March / 1)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
}

TEST_SUITE("iterate train departures 5") {
  TEST_CASE("test trains 5, iterate train departures, all") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 2), ymd_to_abs(2022_y / March / 4),
        ymd_to_abs(2022_y / March / 6), ymd_to_abs(2022_y / March / 8)};

    auto const result = gather_midnights(trains, interval{});

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 5, iterate train departures, none") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 10),
                               .end_ = ymd_to_abs(2022_y / March / 12)};

    std::vector<absolute_time> const result =
        gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 5, iterate train departures, earlier half") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 2)};

    interval const interval = {.start_ = ymd_to_abs(2022_y / February / 15),
                               .end_ = ymd_to_abs(2022_y / March / 4)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 5, iterate train departures, later half") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 4), ymd_to_abs(2022_y / March / 6),
        ymd_to_abs(2022_y / March / 8)};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 4),
                               .end_ = ymd_to_abs(2022_y / March / 14)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE(
      "test trains 5, iterate train departures, two hours with train, start") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 4),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 4) + hours{7},
        .end_ = ymd_to_abs(2022_y / March / 4) + hours{9}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
  TEST_CASE(
      "test trains 5, iterate train departures, two hours with train, middle") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 4),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 4) + hours{10},
        .end_ = ymd_to_abs(2022_y / March / 4) + hours{12}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE(
      "test trains 5, iterate train departures, two hours with train, end") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 4),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 4) + hours{15},
        .end_ = ymd_to_abs(2022_y / March / 4) + hours{17}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE(
      "test trains 5, iterate train departures, max hours without train") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 4) + hours{16} + seconds{1},
        .end_ = ymd_to_abs(2022_y / March / 6) + hours{8} - seconds{1}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE(
      "test trains 5, iterate train departures, min hours with 3 trains") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 4),
        ymd_to_abs(2022_y / March / 6),
        ymd_to_abs(2022_y / March / 8),
    };

    interval const interval = {
        .start_ = ymd_to_abs(2022_y / March / 4) + hours{16},
        .end_ = ymd_to_abs(2022_y / March / 8) + hours{8}};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }

  TEST_CASE("test trains 5, iterate train departures, midnight to midnight") {
    std::vector<train> const trains = {test_train5()};

    std::vector<absolute_time> const expected = {};

    interval const interval = {.start_ = ymd_to_abs(2022_y / March / 5),
                               .end_ = ymd_to_abs(2022_y / March / 6)};

    auto const result = gather_midnights(trains, interval);

    CHECK_EQ(result, expected);
  }
}

TEST_SUITE("iterate train departures 6") {

  TEST_CASE("test trains 6, iterate train departures, all") {
    std::vector<train> const trains = {test_train1(), test_train2(),
                                       test_train3(), test_train4(),
                                       test_train5()};

    std::vector<absolute_time> const expected = {
        ymd_to_abs(2022_y / March / 1), ymd_to_abs(2022_y / March / 1),
        ymd_to_abs(2022_y / March / 1), ymd_to_abs(2022_y / March / 2),
        ymd_to_abs(2022_y / March / 2), ymd_to_abs(2022_y / March / 3),
        ymd_to_abs(2022_y / March / 3), ymd_to_abs(2022_y / March / 4),
        ymd_to_abs(2022_y / March / 4), ymd_to_abs(2022_y / March / 5),
        ymd_to_abs(2022_y / March / 5), ymd_to_abs(2022_y / March / 6),
        ymd_to_abs(2022_y / March / 6), ymd_to_abs(2022_y / March / 7),
        ymd_to_abs(2022_y / March / 7), ymd_to_abs(2022_y / March / 8),
        ymd_to_abs(2022_y / March / 8), ymd_to_abs(2022_y / March / 9),
        ymd_to_abs(2022_y / March / 9), ymd_to_abs(2022_y / March / 9)};

    auto const result = gather_midnights(trains, interval{});

    CHECK_EQ(result, expected);
  }
}

}  // namespace soro::test
