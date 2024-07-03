#include "soro/infrastructure/graph/section.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <span>
#include <string>
#include <vector>

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/transform.hpp"

#include "soro/base/soro_types.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/construction/get_dir.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra {

element::ptr section::first_rising() const { return rising_order_.front(); }
element::ptr section::last_rising() const { return rising_order_.back(); }

element::ptr section::second_rising() const {
  utls::ensure(rising_order_.size() > 2, "need at least one track element");

  // don't check the first and last element. they cannot be the second element
  auto const it = std::find_if(
      std::begin(rising_order_) + 1, std::end(rising_order_) - 1, [](auto&& e) {
        utls::sassert(e->is_track_element(), "non track element found");
        return e->template as<track_element>().rising();
      });

  utls::sassert(it != std::end(rising_order_), "no rising track element found");

  return *it;
}

element::ptr section::second_to_last_rising() const {
  utls::ensure(rising_order_.size() > 2, "need at least one track element");

  // don't check the first and last element. they cannot be the second element
  auto const it = std::find_if(
      std::rbegin(rising_order_) + 1, std::rend(rising_order_) - 1,
      [](auto&& e) {
        utls::sassert(e->is_track_element(), "non track element found");
        return e->template as<track_element>().rising();
      });

  utls::sassert(it != std::rend(rising_order_),
                "no rising track element found");
  return *it;
}

element::ptr section::first_falling() const { return falling_order_.front(); }
element::ptr section::last_falling() const { return falling_order_.back(); }

element::ptr section::opposite_end(element::ptr const from) const {
  return from == first_rising() ? last_rising() : first_rising();
}

soro::size_t section::size() const {
  return utls::narrow<soro::size_t>(rising_order_.size());
}

bool section::is_empty_border_section() const {
  return size() == 2 && first_rising()->is(type::BORDER) &&
         last_rising()->is(type::BORDER);
}

std::span<element::ptr const> section::from(element::ptr const element,
                                            mileage_dir const dir) const {
  auto const get_range = [&](auto const& v) {
    auto const from = std::find(std::begin(v), std::end(v), element);
    return std::span{from, std::end(v)};
  };

  return dir == mileage_dir::rising ? get_range(rising_order_)
                                    : get_range(falling_order_);
}

std::span<element::ptr const> section::to(element::ptr const element,
                                          mileage_dir const dir) const {
  auto const get_range = [&](auto const& v) {
    auto const to = std::find(std::begin(v), std::end(v), element);
    return std::span{std::begin(v), to + 1};
  };

  return dir == mileage_dir::rising ? get_range(rising_order_)
                                    : get_range(falling_order_);
}

std::span<element::ptr const> section::from_to(element::ptr const from,
                                               element::ptr const to,
                                               mileage_dir const dir) const {
  auto const get_range = [&](auto const& v) {
    auto const f = std::find(std::begin(v), std::end(v), from);
    auto const t = std::find(std::begin(v), std::end(v), to);
    return std::span{f, t + 1};
  };

  return dir == mileage_dir::rising ? get_range(rising_order_)
                                    : get_range(falling_order_);
}

si::length section::length() const {
  return (first_rising()->km(rising_order_[1]) -
          last_rising()->km(rising_order_[rising_order_.size() - 2]))
      .abs();
}

bool section::ok() const {
  auto const rising_ids =
      rising_order_ |
      ranges::views::transform([](auto&& e) { return e->get_id(); }) |
      ranges::to<std::set>();

  auto const falling_ids =
      falling_order_ |
      ranges::views::transform([](auto&& e) { return e->get_id(); }) |
      ranges::to<std::set>();

  return rising_ids == falling_ids;
}

section const& sections::operator[](section::id const id) const {
  return sections_[id];
}

section& sections::operator[](section::id const id) { return sections_[id]; }

soro::size_t sections::size() const { return sections_.size(); }

section::id sections::create_section() {
  sections_.resize(sections_.size() + 1);
  return section::id{utls::narrow<section::id::value_t>(sections_.size() - 1)};
}

sections::it sections::begin() const { return std::begin(sections_); }
sections::it sections::end() const { return std::end(sections_); }

void sections::add_section_id_to_element(element const& e,
                                         std::string const& node_name,
                                         section::position const pos,
                                         section::id const section_id) {

  if (element_to_section_ids_.size() <= as_val(e.get_id())) {
    // elements have to be added consecutively with this function,
    // given how it is implemented
    utls::sassert(element_to_section_ids_.size() == as_val(e.get_id()),
                  "trying to add non-consecutive element {}", e.get_id());

    element_to_section_ids_.emplace_back(
        std::vector<section::id>(e.direction_count(), section::invalid()));
  }

  auto const dir_idx = get_dir(node_name, pos);

  utls::sassert(dir_idx < element_to_section_ids_[e.get_id()].size(),
                "idx {} not in range", dir_idx);
  utls::sassert(
      element_to_section_ids_[e.get_id()][dir_idx] == section::invalid(),
      "overwriting a non-invalid section id");

  element_to_section_ids_[e.get_id()][dir_idx] = section_id;
}

bool sections::ok() const {
  auto const all_set =
      utls::all_of(element_to_section_ids_.data_,
                   [](auto&& i) { return i != section::invalid(); });

  if (!all_set) {
    return false;
  }

  auto const all_sections_ok =
      utls::all_of(sections_, [](auto&& s) { return s.ok(); });

  if (!all_sections_ok) {
    return false;
  }

  return true;
}

}  // namespace soro::infra