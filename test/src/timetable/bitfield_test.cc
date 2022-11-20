#include "doctest/doctest.h"

#include "soro/utls/std_wrapper/std_wrapper.h"

#include "soro/timetable/bitfield.h"

using namespace date;

using namespace soro;
using namespace soro::tt;

TEST_SUITE("bitfield") {

  TEST_CASE("construct bitfield") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 1;

    auto const bf = make_bitfield(s, t, "01010101");

    CHECK_EQ(bf.bitset_.count(), 4);
  }

  TEST_CASE("construct bitfield with only a single day") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / February / 22;

    auto const bf = make_bitfield(s, t, "1");

    CHECK_EQ(bf.bitset_.count(), 1);
    CHECK(bf[s]);
    CHECK(bf[t]);
  }

  TEST_CASE("operator[] both ends 1") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits = "101010101";

    auto const bf = make_bitfield(s, t, bits.data());

    year_month_day const s1 = 2022_y / February / 23;
    year_month_day const s2 = 2022_y / February / 24;
    year_month_day const s3 = 2022_y / February / 25;
    year_month_day const s4 = 2022_y / February / 26;
    year_month_day const s5 = 2022_y / February / 27;
    year_month_day const s6 = 2022_y / February / 28;
    year_month_day const s7 = 2022_y / March / 1;
    year_month_day const s8 = 2022_y / March / 2;

    CHECK(bf[s]);
    CHECK(!bf[s1]);
    CHECK(bf[s2]);
    CHECK(!bf[s3]);
    CHECK(bf[s4]);
    CHECK(!bf[s5]);
    CHECK(bf[s6]);
    CHECK(!bf[s7]);
    CHECK(bf[s8]);

    CHECK_EQ(bf.bitset_.count(), utls::count(bits, '1'));
  }

  TEST_CASE("operator[] both ends 0") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits = "010101010";

    auto const bf = make_bitfield(s, t, bits.data());

    year_month_day const s1 = 2022_y / February / 23;
    year_month_day const s2 = 2022_y / February / 24;
    year_month_day const s3 = 2022_y / February / 25;
    year_month_day const s4 = 2022_y / February / 26;
    year_month_day const s5 = 2022_y / February / 27;
    year_month_day const s6 = 2022_y / February / 28;
    year_month_day const s7 = 2022_y / March / 1;
    year_month_day const s8 = 2022_y / March / 2;

    CHECK(!bf[s]);
    CHECK(bf[s1]);
    CHECK(!bf[s2]);
    CHECK(bf[s3]);
    CHECK(!bf[s4]);
    CHECK(bf[s5]);
    CHECK(!bf[s6]);
    CHECK(bf[s7]);
    CHECK(!bf[s8]);

    CHECK_EQ(bf.bitset_.count(), utls::count(bits, '1'));
  }

  TEST_CASE("operator[] 1 - 0") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits = "110101010";

    auto const bf = make_bitfield(s, t, bits.data());

    year_month_day const s1 = 2022_y / February / 23;
    year_month_day const s2 = 2022_y / February / 24;
    year_month_day const s3 = 2022_y / February / 25;
    year_month_day const s4 = 2022_y / February / 26;
    year_month_day const s5 = 2022_y / February / 27;
    year_month_day const s6 = 2022_y / February / 28;
    year_month_day const s7 = 2022_y / March / 1;
    year_month_day const s8 = 2022_y / March / 2;

    CHECK(bf[s]);
    CHECK(bf[s1]);
    CHECK(!bf[s2]);
    CHECK(bf[s3]);
    CHECK(!bf[s4]);
    CHECK(bf[s5]);
    CHECK(!bf[s6]);
    CHECK(bf[s7]);
    CHECK(!bf[s8]);

    CHECK_EQ(bf.bitset_.count(), utls::count(bits, '1'));
  }

  TEST_CASE("operator[] 0 - 1") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits = "010101011";

    auto const bf = make_bitfield(s, t, bits.data());

    year_month_day const s1 = 2022_y / February / 23;
    year_month_day const s2 = 2022_y / February / 24;
    year_month_day const s3 = 2022_y / February / 25;
    year_month_day const s4 = 2022_y / February / 26;
    year_month_day const s5 = 2022_y / February / 27;
    year_month_day const s6 = 2022_y / February / 28;
    year_month_day const s7 = 2022_y / March / 1;
    year_month_day const s8 = 2022_y / March / 2;

    CHECK(!bf[s]);
    CHECK(bf[s1]);
    CHECK(!bf[s2]);
    CHECK(bf[s3]);
    CHECK(!bf[s4]);
    CHECK(bf[s5]);
    CHECK(!bf[s6]);
    CHECK(bf[s7]);
    CHECK(bf[s8]);

    CHECK_EQ(bf.bitset_.count(), utls::count(bits, '1'));
  }

  TEST_CASE("get_set_days") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits = "101010101";

    auto const bf = make_bitfield(s, t, bits.data());

    year_month_day const s2 = 2022_y / February / 24;
    year_month_day const s4 = 2022_y / February / 26;
    year_month_day const s6 = 2022_y / February / 28;
    year_month_day const s8 = 2022_y / March / 2;

    CHECK(bf[s]);
    CHECK(bf[s2]);
    CHECK(bf[s4]);
    CHECK(bf[s6]);
    CHECK(bf[s8]);

    auto const result = bf.get_set_days();

    std::vector<year_month_day> const expected = {s, s2, s4, s6, s8};
    CHECK_EQ(result, expected);
  }

  TEST_CASE("operator |= with self") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits = "101010101";

    auto bf = make_bitfield(s, t, bits.data());
    auto const copy = bf;

    bf |= copy;

    CHECK_EQ(bf, copy);
  }

  TEST_CASE("operator |= same first and last date") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits1 = "101010101";
    std::string const bits2 = "010101010";

    auto const bf1 = make_bitfield(s, t, bits1.data());
    auto const bf2 = make_bitfield(s, t, bits2.data());

    auto const r1 = bf1 | bf2;
    auto const r2 = bf2 | bf1;

    std::string const expected_bits = "111111111";
    auto const expected_result = make_bitfield(s, t, expected_bits.data());

    CHECK_EQ(r1, expected_result);
    CHECK_EQ(r2, expected_result);
  }

  TEST_CASE("operator |= same first and different last date") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    std::string const bits1 = "101010101";
    std::string const bits2 = "01010101000";

    year_month_day const new_t = 2022_y / March / 4;

    auto const bf1 = make_bitfield(s, t, bits1.data());
    auto const bf2 = make_bitfield(s, new_t, bits2.data());

    auto const r1 = bf1 | bf2;
    auto const r2 = bf2 | bf1;

    std::string const expected_bits = "11111111100";
    auto const expected_result = make_bitfield(s, new_t, expected_bits.data());

    CHECK_EQ(r1, expected_result);
    CHECK_EQ(r2, expected_result);
  }

  TEST_CASE("operator |= different first and last date - no gap") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    year_month_day const new_s = 2022_y / March / 3;
    year_month_day const new_t = 2022_y / March / 11;

    std::string const bits1 = "101010101";
    std::string const bits2 = "010101010";

    auto const bf1 = make_bitfield(s, t, bits1.data());
    auto const bf2 = make_bitfield(new_s, new_t, bits2.data());

    auto const r1 = bf1 | bf2;
    auto const r2 = bf2 | bf1;

    std::string const expected_bits = "101010101010101010";
    auto const expected_result = make_bitfield(s, new_t, expected_bits.data());

    CHECK_EQ(r1, expected_result);
    CHECK_EQ(r2, expected_result);
  }

  TEST_CASE("operator |= different first and last date - with gap") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    year_month_day const new_s = 2022_y / March / 15;
    year_month_day const new_t = 2022_y / March / 23;

    std::string const bits1 = "101010101";
    std::string const bits2 = "010101010";

    auto const bf1 = make_bitfield(s, t, bits1.data());
    auto const bf2 = make_bitfield(new_s, new_t, bits2.data());

    auto const r1 = bf1 | bf2;
    auto const r2 = bf2 | bf1;

    std::string const expected_bits = "101010101000000000000010101010";
    auto const expected_result = make_bitfield(s, new_t, expected_bits.data());

    CHECK_EQ(r1, expected_result);
    CHECK_EQ(r2, expected_result);
  }

  TEST_CASE("operator |= different first and last date - with overlap") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    year_month_day const new_s = 2022_y / March / 1;
    year_month_day const new_t = 2022_y / March / 9;

    std::string const bits1 = "101010101";
    std::string const bits2 = "111111111";

    auto const bf1 = make_bitfield(s, t, bits1.data());
    auto const bf2 = make_bitfield(new_s, new_t, bits2.data());

    auto const r1 = bf1 | bf2;
    auto const r2 = bf2 | bf1;

    std::string const expected_bits = "1010101111111111";
    auto const expected_result = make_bitfield(s, new_t, expected_bits.data());

    CHECK_EQ(r1, expected_result);
    CHECK_EQ(r2, expected_result);
  }

  TEST_CASE("set - in range") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    year_month_day const set_date = 2022_y / March / 1;

    std::string const bits = "101010101";

    auto bf = make_bitfield(s, t, bits.data());
    bf.set(set_date, true);

    std::string const expected_bits = "101010111";
    auto const expected_result = make_bitfield(s, t, expected_bits.data());

    CHECK_EQ(bf, expected_result);
  }

  TEST_CASE("set - before range") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    year_month_day const set_date = 2022_y / February / 1;

    std::string const bits = "101010101";

    auto bf = make_bitfield(s, t, bits.data());
    bf.set(set_date, true);

    std::string const expected_bits = "100000000000000000000101010101";
    auto const expected_result =
        make_bitfield(set_date, t, expected_bits.data());

    CHECK_EQ(bf, expected_result);
  }

  TEST_CASE("set - after range") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    year_month_day const set_date = 2022_y / March / 15;

    std::string const bits1 = "101010101";

    auto bf = make_bitfield(s, t, bits1.data());
    bf.set(set_date, true);

    std::string const expected_bits = "1010101010000000000001";
    auto const expected_result =
        make_bitfield(s, set_date, expected_bits.data());

    CHECK_EQ(bf, expected_result);
  }

#if !defined(NDEBUG)
  TEST_CASE("construct bitfield throws - end before start") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    CHECK_THROWS(make_bitfield(t, s, "10101010"));
  }

  TEST_CASE("construct bitfield throws - more than BITSIZE days") {
    year_month_day const s = 2022_y / February / 22;

    year_month_day const t = {static_cast<sys_days>(s) +
                              days{bitfield::BITSIZE + 1}};

    std::string const bits(bitfield::BITSIZE + 1, '0');

    CHECK_THROWS(make_bitfield(s, t, bits.data()));
  }

  TEST_CASE("out of date access throws") {
    year_month_day const s = 2022_y / February / 22;
    year_month_day const t = 2022_y / March / 2;

    auto const bf = make_bitfield(s, t, "101010101");
    CHECK_THROWS(bf[2022_y / February / 21]);
  }
#endif
}
