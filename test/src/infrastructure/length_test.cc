#include "doctest/doctest.h"

#include "utl/timer.h"

#include "test/file_paths.h"

#include "soro/infrastructure/path/length.h"
#include "soro/timetable/timetable.h"

using namespace soro;
using namespace infra;
using namespace tt;

// Generally, sections should not have a length of zero.
// There is an exception if both section elements are on the same mileage.
// This checks if this exception applies to the given section.
void test_section_with_zero_length(section const& sec) {
  CHECK(sec.elements_.size() == 2);
  auto const first = sec.elements_.front();
  auto const last = sec.elements_.back();
  CHECK(first->get_km(last) == last->get_km(first));
}

void test_section_lengths(infrastructure const& infra) {
  for (auto const& sec : infra->graph_.sections_) {

    {  // Check rising path
      auto const l1 = get_path_length_from_elements(sec.iterate(rising::YES));
      auto const l2 = get_path_length_from_sections(sec.iterate(rising::YES));

      CHECK_MESSAGE(l1 == l2,
                    "Different lengths from the two length calculation funs");

      if (l1 == si::ZERO<si::length>) {
        test_section_with_zero_length(sec);
      }
    }

    {  // Check falling path
      auto const l1 = get_path_length_from_elements(sec.iterate(rising::NO));
      auto const l2 = get_path_length_from_sections(sec.iterate(rising::NO));

      CHECK_MESSAGE(l1 == l2,
                    "Different lengths from the two length calculation funs");

      if (l1 == si::ZERO<si::length>) {
        test_section_with_zero_length(sec);
      }
    }
  }
}

void test_station_route_lengths(infrastructure const& infra) {
  for (auto const& sr : infra->station_routes_) {
    auto const e1 = get_path_length_from_elements(sr->nodes());
    auto const e2 = get_path_length_from_elements(utls::coro_map(
        sr->entire(skip_omitted::OFF), [](auto&& rn) { return rn.node_; }));
    auto const e3 = get_path_length_from_elements(utls::coro_map(
        sr->entire(skip_omitted::ON), [](auto&& rn) { return rn.node_; }));

    CHECK(e1 == e2);
    CHECK(e2 == e3);

    auto const s1 = get_path_length_from_sections(sr->nodes());
    auto const s2 = get_path_length_from_sections(utls::coro_map(
        sr->entire(skip_omitted::OFF), [](auto&& rn) { return rn.node_; }));
    auto const s3 = get_path_length_from_sections(utls::coro_map(
        sr->entire(skip_omitted::ON), [](auto&& rn) { return rn.node_; }));

    CHECK(s1 == s2);
    CHECK(s2 == s3);

    CHECK_MESSAGE(e1 == s2,
                  "Different lengths from the two length calculation funs");
  }
}

void test_interlocking_route_lengths(infrastructure const&) {
  utls::sassert(false, "Not implemented");
  //  for (auto const& ir : infra->interlocking_.interlocking_routes_) {
  //    auto const e1 = get_path_length_from_elements(ir->nodes());
  //    auto const e2 = get_path_length_from_elements(utls::coro_map(
  //        ir->entire(skip_omitted::OFF), [](auto&& rn) { return rn.node_; }));
  //    auto const e3 = get_path_length_from_elements(utls::coro_map(
  //        ir->entire(skip_omitted::ON), [](auto&& rn) { return rn.node_; }));
  //
  //    CHECK(e1 == e2);
  //    CHECK(e2 == e3);
  //
  //    auto const s1 = get_path_length_from_sections(ir->nodes());
  //    auto const s2 = get_path_length_from_sections(utls::coro_map(
  //        ir->entire(skip_omitted::OFF), [](auto&& rn) { return rn.node_; }));
  //    auto const s3 = get_path_length_from_sections(utls::coro_map(
  //        ir->entire(skip_omitted::ON), [](auto&& rn) { return rn.node_; }));
  //
  //    CHECK(s1 == s2);
  //    CHECK(s2 == s3);
  //
  //    CHECK_MESSAGE(e1 == s1,
  //                  "Different lengths from the two length calculation funs");
  //  }
}

void test_train_path_lengths(timetable const& tt) {
  for (auto const& train : tt) {
    auto const e1 = get_path_length_from_elements(utls::coro_map(
        train->iterate(skip_omitted::OFF), [](auto&& rn) { return rn.node_; }));

    auto const s1 = get_path_length_from_sections(utls::coro_map(
        train->iterate(skip_omitted::ON), [](auto&& rn) { return rn.node_; }));

    CHECK_MESSAGE(s1 == e1,
                  "Different lengths from the two length calculation funs");
  }
}

TEST_SUITE("path length") {
  TEST_CASE("section lengths") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    test_section_lengths(infra);
  }

  TEST_CASE("station route lengths") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    test_station_route_lengths(infra);
  }

  TEST_CASE("interlocking route lengths") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    test_interlocking_route_lengths(infra);
  }

  TEST_CASE("train path lengths") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(FOLLOW_OPTS, infra);
    test_train_path_lengths(tt);
  }
}
