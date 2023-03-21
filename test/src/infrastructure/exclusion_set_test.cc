#include "doctest/doctest.h"

#include "soro/infrastructure/exclusion/exclusion_set.h"

using namespace soro::infra;

void check_set(exclusion_set const& set) { CHECK(set.ok()); }

TEST_SUITE("exclusion set") {
  TEST_CASE("construct success") {
    soro::vector<uint32_t> ids = {1, 2, 3, 4, 5};
    auto es = make_exclusion_set(ids);

    CHECK_EQ(es.first_, 0U);
    CHECK_EQ(es.last_, exclusion_set::bitvec_t::bits_per_block - 1);

    CHECK_EQ(es.first_bit_set_, ids.front());
    CHECK_EQ(es.last_bit_set_, ids.back());
    CHECK(!es.empty());

    es.clear();

    CHECK(es.empty());
    check_set(es);
  }

#if !defined(NDEBUG)
  TEST_CASE("construct failure - non sorted") {
    soro::vector<uint32_t> const ids = {6, 1, 2, 3, 4, 5};
    CHECK_THROWS(make_exclusion_set(ids));
  }
#endif

  TEST_CASE("contains - identity") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto const es1 = make_exclusion_set(ids);

    CHECK(es1.contains(es1));
    CHECK_EQ(es1.compare(es1), std::partial_ordering::equivalent);
    check_set(es1);
  }

  TEST_CASE("contains - same") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto const es1 = make_exclusion_set(ids);
    auto const es2 = make_exclusion_set(ids);

    CHECK(es1.contains(es2));
    CHECK(es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::equivalent);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::equivalent);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - empty") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto const es = make_exclusion_set(ids);

    auto const empty = make_exclusion_set({});

    CHECK(es.contains(empty));
    CHECK(!empty.contains(es));
    CHECK_EQ(es.compare(empty), std::partial_ordering::greater);
    CHECK_EQ(empty.compare(es), std::partial_ordering::less);
    check_set(es);
    check_set(empty);
  }

  TEST_CASE("contains - partial 1") {
    soro::vector<uint32_t> const ids1 = {1, 2, 3, 4, 5};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {2, 3, 4};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::greater);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::less);

    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - partial 2") {
    soro::vector<uint32_t> const ids1 = {1, 2, 3, 4, 5};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {1, 5};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::greater);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::less);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - partial 3") {
    soro::vector<uint32_t> const ids1 = {1, 2, 3, 4, 5};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {1, 2, 3};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::greater);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::less);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - partial 4") {
    soro::vector<uint32_t> const ids1 = {1, 2, 3, 4, 5};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {3, 4, 5};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::greater);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::less);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - holes 1") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {5, 10'001};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::greater);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::less);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - holes 2") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {5, 10'002};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(!es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::unordered);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::unordered);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - holes 3") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {5, 10'000, 80'000, 100'000};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(!es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::unordered);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::unordered);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - holes 4") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {900, 10'000, 80'000, 100'000};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(!es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::unordered);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::unordered);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("contains - holes 5") {
    soro::vector<uint32_t> const ids1 = {0, 2048};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {1024};
    auto const es2 = make_exclusion_set(ids2);

    CHECK(!es1.contains(es2));
    CHECK(!es2.contains(es1));
    CHECK_EQ(es1.compare(es2), std::partial_ordering::unordered);
    CHECK_EQ(es2.compare(es1), std::partial_ordering::unordered);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("expanded set") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto const es = make_exclusion_set(ids);

    auto const result = es.expanded_set();

    CHECK_EQ(result, ids);
    check_set(es);
  }

  TEST_CASE("expanded set - holes") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5, 10'000, 10'001, 100'000};
    auto const es = make_exclusion_set(ids);

    auto const result = es.expanded_set();

    CHECK_EQ(result, ids);
    check_set(es);
  }

  TEST_CASE("iterator") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto const es = make_exclusion_set(ids);

    soro::vector<uint32_t> const result(std::begin(es), std::end(es));

    CHECK_EQ(result, ids);
    check_set(es);
  }

  TEST_CASE("iterator - holes") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5, 10'000, 10'001, 100'000};
    auto const es = make_exclusion_set(ids);

    soro::vector<uint32_t> const result(std::begin(es), std::end(es));

    CHECK_EQ(result, ids);
    check_set(es);
  }

  TEST_CASE("iterator - first bit larger than size") {
    soro::vector<uint32_t> const ids = {10'000, 10'001};
    auto const es = make_exclusion_set(ids);

    soro::vector<uint32_t> const result(std::begin(es), std::end(es));

    CHECK_EQ(result, ids);
    check_set(es);
  }

  TEST_CASE("union - self") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto es1 = make_exclusion_set(ids);
    auto es2 = make_exclusion_set(ids);

    es1 |= es2;

    CHECK_EQ(es1.expanded_set(), ids);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("union - empty") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto es1 = make_exclusion_set(ids);
    auto es2 = make_exclusion_set({});

    auto const merged1 = es1 | es2;
    auto const merged2 = es2 | es1;

    CHECK_EQ(merged1.expanded_set(), ids);
    CHECK_EQ(merged2.expanded_set(), ids);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("union - holes 1") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {5, 10'001};
    auto const es2 = make_exclusion_set(ids2);

    auto result1 = make_exclusion_set(ids1);
    result1 |= es2;

    auto result2 = make_exclusion_set(ids2);
    result2 |= es1;

    CHECK_EQ(result1.expanded_set(), ids1);
    CHECK_EQ(result2.expanded_set(), ids1);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("union - holes 2") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {5, 10'002};
    auto const es2 = make_exclusion_set(ids2);

    auto const merged1 = es1 | es2;
    auto const merged2 = es2 | es1;

    soro::vector<uint32_t> const expected = {1,      2,      3,      4,      5,
                                             10'000, 10'001, 10'002, 100'000};

    auto const result1 = merged1.expanded_set();
    auto const result2 = merged2.expanded_set();

    CHECK_EQ(result1, expected);
    CHECK_EQ(result2, expected);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("union - holes 3") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {70'000, 75'000, 75'001};
    auto const es2 = make_exclusion_set(ids2);

    auto const merged1 = es1 | es2;
    auto const merged2 = es2 | es1;

    auto const result1 = merged1.expanded_set();
    auto const result2 = merged2.expanded_set();

    soro::vector<uint32_t> const expected = {
        1, 2, 3, 4, 5, 10'000, 10'001, 70'000, 75'000, 75'001, 100'000};

    CHECK_EQ(result1, expected);
    CHECK_EQ(result2, expected);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - self") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto es1 = make_exclusion_set(ids);
    auto es2 = make_exclusion_set(ids);

    auto const subtracted = es1 - es2;

    auto const result = subtracted.expanded_set();

    CHECK(result.empty());
    CHECK(subtracted.empty());
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - empty") {
    soro::vector<uint32_t> const ids = {1, 2, 3, 4, 5};
    auto es1 = make_exclusion_set(ids);
    auto es2 = make_exclusion_set({});

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    CHECK_EQ(result1, ids);
    CHECK(result2.empty());
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - first") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {1, 10'001, 200'000};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    soro::vector<uint32_t> const expected1 = {2, 3, 4, 5, 10'000, 100'000};
    soro::vector<uint32_t> const expected2 = {200'000};

    CHECK_EQ(result1, expected1);
    CHECK_EQ(result2, expected2);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - last") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {10'002, 100'000};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    soro::vector<uint32_t> const expected1 = {1, 2, 3, 4, 5, 10'000, 10'001};
    soro::vector<uint32_t> const expected2 = {10'002};

    CHECK_EQ(result1, expected1);
    CHECK_EQ(result2, expected2);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - both") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {1, 10'001, 10'002, 100'000};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    soro::vector<uint32_t> const expected1 = {2, 3, 4, 5, 10'000};
    soro::vector<uint32_t> const expected2 = {10'002};

    CHECK_EQ(result1, expected1);
    CHECK_EQ(result2, expected2);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - holes 1") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {5, 10'001};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    soro::vector<uint32_t> const expected = {1, 2, 3, 4, 10'000, 100'000};

    CHECK_EQ(result1, expected);
    CHECK(result2.empty());
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - holes 2") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {5, 10'002};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    soro::vector<uint32_t> const expected1 = {1,      2,      3,      4,
                                              10'000, 10'001, 100'000};
    soro::vector<uint32_t> const expected2 = {10'002};

    CHECK_EQ(result1, expected1);
    CHECK_EQ(result2, expected2);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - holes 3") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {70'000, 75'000, 75'001};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    CHECK_EQ(result1, ids1);
    CHECK_EQ(result2, ids2);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - partial overlap front no matches") {
    soro::vector<uint32_t> const ids1 = {1, 2,      3,      4,
                                         5, 10'000, 10'001, 100'000};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {700, 75'000, 75'001, 200'000, 200'001};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    CHECK_EQ(result1, ids1);
    CHECK_EQ(result2, ids2);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - partial overlap matches") {
    soro::vector<uint32_t> const ids1 = {
        1, 2, 3, 4, 5, 10'000, 75'000, 75'001, 100'000, 100'001};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {700, 75'000, 75'001, 200'000, 200'001};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    soro::vector<uint32_t> const expected1 = {1, 2,      3,       4,
                                              5, 10'000, 100'000, 100'001};
    soro::vector<uint32_t> const expected2 = {700, 200'000, 200'001};

    CHECK_EQ(result1, expected1);
    CHECK_EQ(result2, expected2);
    check_set(es1);
    check_set(es2);
  }

  TEST_CASE("subtract - no overlap") {
    soro::vector<uint32_t> const ids1 = {10'000, 11'000, 11'001, 11'002};
    auto const es1 = make_exclusion_set(ids1);

    soro::vector<uint32_t> const ids2 = {75'000, 75'001, 200'000, 200'001};
    auto const es2 = make_exclusion_set(ids2);

    auto const subtracted1 = es1 - es2;
    auto const subtracted2 = es2 - es1;

    auto const result1 = subtracted1.expanded_set();
    auto const result2 = subtracted2.expanded_set();

    CHECK_EQ(result1, ids1);
    CHECK_EQ(result2, ids2);
    check_set(es1);
    check_set(es2);
  }
}
