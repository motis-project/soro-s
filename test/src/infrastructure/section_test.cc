#include "test/infrastructure/section_test.h"

#include "doctest/doctest.h"

#include "soro/utls/coroutine/collect.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/is_path.h"
#include "soro/infrastructure/path/length.h"

using namespace soro;
using namespace infra;

namespace soro::infra::test {

void sections_are_paths(section const& sec) {
  CHECK(is_path(sec.iterate(rising::YES, true)));
  CHECK(is_path(sec.iterate(rising::NO, true)));
}

void section_iterators_have_all_elements(section const& sec) {
  auto const rising_elements =
      utls::collect<std::vector<element::ptr>>(sec.iterate(rising::YES, true));
  auto const falling_elements =
      utls::collect<std::vector<element::ptr>>(sec.iterate(rising::NO, true));

  std::set<element::ptr> iterated_elements;
  for (auto const& e : rising_elements) {
    iterated_elements.insert(e);
  }

  for (auto const& e : falling_elements) {
    iterated_elements.insert(e);
  }

  std::set<element::ptr> expected_elements;
  for (auto const& e : sec.elements_) {
    expected_elements.insert(e);
  }

  std::size_t const undirected_count = utls::count_if(
      sec.elements_, [](auto&& e) { return e->is_undirected_track_element(); });

  std::size_t const iterated_element_count = (rising_elements.size() - 2) +
                                             (falling_elements.size() - 2) + 2 -
                                             undirected_count;
  CHECK_EQ(iterated_element_count, sec.elements_.size());

  CHECK_EQ(iterated_elements, expected_elements);
}

void check_section_element_types(section const& section) {
  auto const total_track_elements = utls::count_if(
      section.elements_, [](auto&& e) { return e->is_track_element(); });

  CHECK_MESSAGE((total_track_elements == section.elements_.size() - 2),
                "Only the first and the last element are non-track elements "
                "(=section elements)");
  CHECK_MESSAGE(!section.elements_.front()->is_track_element(),
                "Only the first and the last element are non-track elements "
                "(=section elements)");
  CHECK_MESSAGE(!section.elements_.back()->is_track_element(),
                "Only the first and the last element are non-track elements "
                "(=section elements)");
}

void check_section_increasing_kmp(section const& section) {
  for (auto const [e1, e2] : utl::pairwise(section.elements_)) {
    // it is safe to pass a nullptr here, since every section only
    // contains two section elements, the first and the last element of the
    // section
    auto const km1 =
        e1->is_section_element() ? e1->get_km(e2) : e1->get_km(nullptr);
    auto const km2 =
        e2->is_section_element() ? e2->get_km(e1) : e2->get_km(nullptr);

    CHECK_MESSAGE(
        (km1 <= km2),
        "Elements in section not in increasing kilometerpoint order.");
  }
}

void check_section_length(section const& sec) {
  {  // Check rising path
    auto const l1 =
        get_path_length_from_elements(sec.iterate(rising::YES, true));
    auto const l2 =
        get_path_length_from_sections(sec.iterate(rising::YES, true));

    CHECK_MESSAGE((l1 == l2),
                  "Different lengths from the two length calculation funs");
  }

  {  // Check falling path
    auto const l1 =
        get_path_length_from_elements(sec.iterate(rising::NO, true));
    auto const l2 =
        get_path_length_from_sections(sec.iterate(rising::NO, true));

    CHECK_MESSAGE((l1 == l2),
                  "Different lengths from the two length calculation funs");
  }
}

void check_section_iteration_direction(section const& sec) {
  auto const rising_elements =
      utls::collect<std::vector<element::ptr>>(sec.iterate(rising::YES, true));

  for (auto const& [from, to] : utl::pairwise(rising_elements)) {
    CHECK_LE(from->get_km(to), to->get_km(from));
    CHECK_GE(to->get_km(from), from->get_km(to));
  }

  auto const falling_elements =
      utls::collect<std::vector<element::ptr>>(sec.iterate(rising::NO, true));
  for (auto const& [from, to] : utl::pairwise(falling_elements)) {
    CHECK_GE(from->get_km(to), to->get_km(from));
    CHECK_LE(to->get_km(from), from->get_km(to));
  }
}

void check_section(section const& sec) {
  sections_are_paths(sec);
  section_iterators_have_all_elements(sec);
  check_section_increasing_kmp(sec);
  check_section_element_types(sec);
  check_section_length(sec);
  check_section_iteration_direction(sec);
}

void do_section_tests(soro::vector<section> const& sections) {
  for (auto const& section : sections) {
    check_section(section);
  }
}

}  // namespace soro::infra::test
