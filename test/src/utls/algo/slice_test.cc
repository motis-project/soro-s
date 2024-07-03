#include "doctest/doctest.h"

#include <iterator>

#include "soro/base/soro_types.h"

#include "soro/utls/algo/slice.h"
#include "soro/utls/std_wrapper/copy.h"

using namespace soro;

TEST_SUITE("slice suite") {
  TEST_CASE("slice simple") {
    soro::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    utls::slice(v, 3, 8);
    soro::vector<int> const expected = {4, 5, 6, 7, 8};
    CHECK_EQ(v, expected);
  }

  TEST_CASE("slice simple 2") {
    soro::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    utls::slice(v, 0, v.size());
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(v, expected);
  }

  TEST_CASE("slice copy simple") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto const slice = utls::slice(v, 3, 8);
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    soro::vector<int> const expected_slice = {4, 5, 6, 7, 8};

    CHECK_EQ(v, expected);
    CHECK_EQ(slice, expected_slice);
  }

  TEST_CASE("slice copy simple 2") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto const slice = utls::slice(v, 0, v.size());
    CHECK_EQ(v, slice);
  }

  TEST_CASE("slice clasp simple") {
    soro::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    utls::slice_clasp(v, 3, 8);
    soro::vector<int> const expected = {4, 5, 6, 7, 8};
    CHECK_EQ(v, expected);
  }

  TEST_CASE("slice clasp simple 2") {
    soro::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    utls::slice(v, 0, v.size());
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(v, expected);
  }

  TEST_CASE("slice clasp simple 3") {
    soro::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    utls::slice_clasp(v, v.size() - (v.size() + 1), 8);
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8};
    CHECK_EQ(v, expected);
  }

  TEST_CASE("slice clasp simple 4") {
    soro::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    utls::slice_clasp(v, v.size() - (v.size() + 1), v.size() + 1);
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(v, expected);
  }

  TEST_CASE("slice clasp copy simple") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto const slice = utls::slice_clasp(v, 3, 8);
    soro::vector<int> const expected = {4, 5, 6, 7, 8};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice clasp copy simple 2") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto const slice = utls::slice(v, 0, v.size());
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice clasp copy simple 3") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto const slice = utls::slice_clasp(v, v.size() - (v.size() + 1), 8);
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice clasp copy simple 4") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto const slice =
        utls::slice_clasp(v, v.size() - (v.size() + 1), v.size() + 1);
    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice range simple") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    soro::vector<int> slice;

    utls::copy(utls::slice_range(v, 3, 8), std::back_inserter(slice));

    soro::vector<int> const expected = {4, 5, 6, 7, 8};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice range simple 2") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    soro::vector<int> slice;

    utls::copy(utls::slice_range(v, 0, v.size()), std::back_inserter(slice));

    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice clasp range simple") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    soro::vector<int> slice;

    utls::copy(utls::slice_clasp_range(v, 3, 8), std::back_inserter(slice));

    soro::vector<int> const expected = {4, 5, 6, 7, 8};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice clasp range simple 2") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    soro::vector<int> slice;

    utls::copy(utls::slice_clasp_range(v, 0, v.size()),
               std::back_inserter(slice));

    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice clasp range simple 3") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    soro::vector<int> slice;

    utls::copy(utls::slice_clasp_range(v, v.size() - (v.size() + 1), 8),
               std::back_inserter(slice));

    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8};
    CHECK_EQ(slice, expected);
  }

  TEST_CASE("slice clasp range simple 4") {
    soro::vector<int> const v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    soro::vector<int> slice;

    utls::copy(
        utls::slice_clasp_range(v, v.size() - (v.size() + 1), v.size() + 1),
        std::back_inserter(slice));

    soro::vector<int> const expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK_EQ(slice, expected);
  }
}
