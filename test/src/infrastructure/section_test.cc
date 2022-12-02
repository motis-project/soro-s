#include "test/infrastructure/section_test.h"

#include "doctest/doctest.h"

#include "utl/get_or_create.h"

#include "soro/utls/coroutine/collect.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/is_path.h"
#include "soro/infrastructure/path/length.h"

using namespace soro;
using namespace infra;

namespace soro::infra::test {

void sections_are_paths(section const& sec) {
  CHECK(is_path(sec.iterate<direction::Rising, skip::Yes>()));
  CHECK(is_path(sec.iterate<direction::Falling, skip::Yes>()));
}

void section_iterators_have_all_elements(section const& sec) {
  auto const rising_elements = utls::collect<std::vector<element::ptr>>(
      sec.iterate<direction::Rising>());
  auto const falling_elements = utls::collect<std::vector<element::ptr>>(
      sec.iterate<direction::Falling>());

  std::set<element::ptr> iterated_elements;
  for (auto const& e : rising_elements) {
    iterated_elements.insert(e);
  }

  for (auto const& e : falling_elements) {
    iterated_elements.insert(e);
  }

  std::set<element::ptr> expected_elements;
  for (auto const& e : sec.rising_order_) {
    expected_elements.insert(e);
  }

  for (auto const& e : sec.falling_order_) {
    expected_elements.insert(e);
  }

  std::size_t const undirected_count_rising =
      utls::count_if(sec.iterate<direction::Rising>(),
                     [](auto&& e) { return e->is_undirected_track_element(); });

  std::size_t const undirected_count_falling =
      utls::count_if(sec.iterate<direction::Falling>(),
                     [](auto&& e) { return e->is_undirected_track_element(); });

  CHECK_EQ(undirected_count_rising, undirected_count_falling);

  std::size_t const iterated_element_count = (rising_elements.size() - 2) +
                                             (falling_elements.size() - 2) + 2 -
                                             undirected_count_rising;

  CHECK_EQ(iterated_element_count, sec.rising_order_.size());
  CHECK_EQ(iterated_elements, expected_elements);
}

void check_section_element_types(section const& section) {
  auto const total_track_elements_rising =
      utls::count_if(section.iterate<direction::Rising, skip::Yes>(),
                     [](auto&& e) { return e->is_directed_track_element(); });

  auto const total_track_elements_falling =
      utls::count_if(section.iterate<direction::Falling, skip::Yes>(),
                     [](auto&& e) { return e->is_directed_track_element(); });

  auto const total_undirected =
      utls::count_if(section.iterate<direction::Falling, skip::Yes>(),
                     [](auto&& e) { return e->is_undirected_track_element(); });

  auto const total_track_elements = total_track_elements_falling +
                                    total_track_elements_rising +
                                    total_undirected;

  // Only the first and the last element are non-track elements (=section
  // elements)
  CHECK_EQ(total_track_elements, section.falling_order_.size() - 2);
  CHECK_EQ(total_track_elements, section.rising_order_.size() - 2);
  CHECK(!section.first_rising()->is_track_element());
  CHECK(!section.first_falling()->is_track_element());
  CHECK(!section.last_rising()->is_track_element());
  CHECK(!section.last_falling()->is_track_element());
}

void check_section_increasing_kmp(section const& section) {
  auto generator = section.iterate<direction::Rising>();

  auto it = std::begin(generator);
  auto last_element = *it;
  ++it;

  for (; it != std::end(generator); ++it) {
    auto element = *it;

    // it is safe to pass a nullptr here, since every section only
    // contains two section elements, the first and the last element of the
    // section
    auto const km1 = last_element->is_section_element()
                         ? last_element->get_km(element)
                         : last_element->get_km(nullptr);
    auto const km2 = element->is_section_element()
                         ? element->get_km(last_element)
                         : element->get_km(nullptr);

    CHECK_MESSAGE(
        (km1 <= km2),
        "Elements in section not in increasing kilometerpoint order.");

    last_element = element;
  }
}

void check_section_length(section const& sec) {
  {  // Check rising path
    auto const l1 =
        get_path_length_from_elements(sec.iterate<direction::Rising>());
    auto const l2 =
        get_path_length_from_sections(sec.iterate<direction::Rising>());

    CHECK_MESSAGE((l1 == l2),
                  "Different lengths from the two length calculation funs");
  }

  {  // Check falling path
    auto const l1 =
        get_path_length_from_elements(sec.iterate<direction::Falling>());
    auto const l2 =
        get_path_length_from_sections(sec.iterate<direction::Falling>());

    CHECK_MESSAGE((l1 == l2),
                  "Different lengths from the two length calculation funs");
  }
}

void check_section_iteration_direction(section const& sec) {
  auto const rising_elements = utls::collect<std::vector<element::ptr>>(
      sec.iterate<direction::Rising>());

  for (auto const [from, to] : utl::pairwise(rising_elements)) {
    CHECK_LE(from->get_km(to), to->get_km(from));
    CHECK_GE(to->get_km(from), from->get_km(to));
  }

  auto const falling_elements = utls::collect<std::vector<element::ptr>>(
      sec.iterate<direction::Falling>());
  for (auto const [from, to] : utl::pairwise(falling_elements)) {
    CHECK_GE(from->get_km(to), to->get_km(from));
    CHECK_LE(to->get_km(from), from->get_km(to));
  }
}

void check_track_elements_are_ordered_correctly(section const& section) {
  std::map<type, uint8_t> sorted_map{
      {type::TUNNEL, 0},
      {type::SLOPE, 1},
      {type::ENTRY, 2},
      {type::RUNTIME_CHECKPOINT_UNDIRECTED, 3},
      {type::RUNTIME_CHECKPOINT, 4},
      {type::HALT, 5},
      {type::MAIN_SIGNAL, 6},
      {type::APPROACH_SIGNAL, 7},
      {type::PROTECTION_SIGNAL, 8},
      {type::EOTD, 9},
      {type::CTC, 10},
      {type::SPEED_LIMIT, 11},
      {type::FORCED_HALT, 12},
      {type::POINT_SPEED, 13},
      {type::BRAKE_PATH, 14},
      {type::TRACK_NAME, 15},
      {type::LEVEL_CROSSING, 16},
  };

  auto const check_order = [&]<direction Dir>(struct section const& sec) {
    std::map<kilometrage, std::vector<element::ptr>> same_kilometrages;

    for (auto const element : sec.iterate<Dir>()) {
      if (element->is_undirected_track_element()) {
        auto const km = element->template as<undirected_track_element>().km_;
        utl::get_or_create(same_kilometrages, km, [&]() {
          return std::vector<element::ptr>{};
        }).emplace_back(element);
      }

      if (element->is_directed_track_element()) {
        auto const km = element->template as<track_element>().km_;
        utl::get_or_create(same_kilometrages, km, [&]() {
          return std::vector<element::ptr>{};
        }).emplace_back(element);
      }
    }

    for (auto const& [_, elements] : same_kilometrages) {
      if (elements.size() == 1) {
        continue;
      }

      CHECK(utls::is_sorted(elements, [&](auto&& e1, auto&& e2) {
        return sorted_map[e1->type()] < sorted_map[e2->type()];
      }));
    }
  };

  check_order.operator()<direction::Rising>(section);
  check_order.operator()<direction::Falling>(section);
}

void check_section(section const& sec) {
  sections_are_paths(sec);
  section_iterators_have_all_elements(sec);
  check_section_increasing_kmp(sec);
  check_section_element_types(sec);
  check_section_length(sec);
  check_section_iteration_direction(sec);
  check_track_elements_are_ordered_correctly(sec);
}

void do_section_tests(soro::vector<section> const& sections) {
  for (auto const& section : sections) {
    check_section(section);
  }
}

}  // namespace soro::infra::test
