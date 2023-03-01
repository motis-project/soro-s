#include "doctest/doctest.h"

#include "soro/utls/algo/multi_set_merge.h"

using namespace soro::utls;

TEST_SUITE("multi set merge") {
  template <typename Vec, typename... Vecs>
  auto create_ranges(Vec && vec, Vecs && ... vecs) {
    auto const vec_to_pair = [](auto&& v) {
      return std::pair{std::begin(v), std::end(v)};
    };

    auto result = std::vector{vec_to_pair(vec)};
    (result.emplace_back(vec_to_pair(vecs)), ...);

    return result;
  }

  TEST_CASE("single set") {
    std::vector<uint32_t> const s1 = {1, 2, 3, 4};

    auto const ranges = create_ranges(s1);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    CHECK_EQ(s1, result);
  }

  TEST_CASE("same set") {
    std::vector<uint32_t> const s1 = {1, 2, 3, 4};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4};

    auto const ranges = create_ranges(s1, s2);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    CHECK_EQ(s1, result);
  }

  TEST_CASE("different set 1") {
    std::vector<uint32_t> const s1 = {1, 2, 3, 4};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4, 5};

    auto const ranges = create_ranges(s1, s2);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1, 2, 3, 4, 5};

    CHECK_EQ(result, expected);
  }

  TEST_CASE("different set 2") {
    std::vector<uint32_t> const s1 = {1, 2, 3, 4, 5};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4};

    auto const ranges = create_ranges(s1, s2);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1, 2, 3, 4, 5};

    CHECK_EQ(result, expected);
  }

  TEST_CASE("many same set") {
    std::vector<uint32_t> const s1 = {1, 2, 3, 4, 5};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4};
    std::vector<uint32_t> const s3 = {1, 2, 3, 4};
    std::vector<uint32_t> const s4 = {1, 2, 3, 4};
    std::vector<uint32_t> const s5 = {1, 2, 3, 4};
    std::vector<uint32_t> const s6 = {1, 2, 3, 4};
    std::vector<uint32_t> const s7 = {1, 2, 3, 4};
    std::vector<uint32_t> const s8 = {1, 2, 3, 4};
    std::vector<uint32_t> const s9 = {1, 2, 3, 4};

    auto const ranges = create_ranges(s1, s2, s3, s4, s5, s6, s7, s8, s9);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1, 2, 3, 4, 5};

    CHECK_EQ(result, expected);
  }

  TEST_CASE("different no overlap 1") {
    std::vector<uint32_t> const s1 = {1, 2, 3, 4, 5};
    std::vector<uint32_t> const s2 = {100, 101, 102};

    auto const ranges = create_ranges(s1, s2);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1, 2, 3, 4, 5, 100, 101, 102};

    CHECK_EQ(result, expected);
  }

  TEST_CASE("different no overlap 2") {
    std::vector<uint32_t> const s1 = {100, 101, 102};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4, 5};

    auto const ranges = create_ranges(s1, s2);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1, 2, 3, 4, 5, 100, 101, 102};

    CHECK_EQ(result, expected);
  }

  TEST_CASE("different no overlap 3") {
    std::vector<uint32_t> const s1 = {100, 101, 102};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4, 5};
    std::vector<uint32_t> const s3 = {1000, 1001, 1002};

    auto const ranges = create_ranges(s1, s2, s3);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1,   2,   3,    4,    5,   100,
                                            101, 102, 1000, 1001, 1002};

    CHECK_EQ(result, expected);
  }

  TEST_CASE("overlap") {
    std::vector<uint32_t> const s1 = {5, 100, 101, 102};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4, 5};
    std::vector<uint32_t> const s3 = {102, 1000, 1001, 1002};

    auto const ranges = create_ranges(s1, s2, s3);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1,   2,   3,    4,    5,   100,
                                            101, 102, 1000, 1001, 1002};

    CHECK_EQ(result, expected);
  }

  TEST_CASE("overlap 2") {
    std::vector<uint32_t> const s1 = {5, 100, 101, 102};
    std::vector<uint32_t> const s2 = {1, 2, 3, 4, 5};
    std::vector<uint32_t> const s3 = {102, 1000, 1001, 1002};
    std::vector<uint32_t> const s4 = {1,   2,   3,    4,    5,   100,
                                      101, 102, 1000, 1001, 1002};

    auto const ranges = create_ranges(s1, s2, s3, s4);
    auto const result = multi_set_merge<std::vector<uint32_t>>(ranges);

    std::vector<uint32_t> const expected = {1,   2,   3,    4,    5,   100,
                                            101, 102, 1000, 1001, 1002};

    CHECK_EQ(result, expected);
  }
}
