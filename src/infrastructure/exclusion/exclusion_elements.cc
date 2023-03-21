#include "soro/infrastructure/exclusion/exclusion_elements.h"

#include "range/v3/to_container.hpp"
#include "range/v3/view/filter.hpp"
#include "range/v3/view/transform.hpp"

#include "utl/concat.h"
#include "utl/erase.h"
#include "utl/erase_duplicates.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

bool is_exclusion_element(element::ptr e) {
  return e->joins_tracks() ||
         interlocking_route::valid_ends().contains(e->type());
}

template <typename Range>
auto filter_transform_collect(Range&& r) {
  return r | ranges::views::filter(is_exclusion_element) |
         ranges::views::transform([](auto&& e) { return e->id(); }) |
         ranges::to<soro::vector<element_id>>();
}

element::ids get_exclusion_elements(section const& section) {
  return filter_transform_collect(section.rising_order_);
}

element::ids get_exclusion_elements_from(section const& section,
                                         element::ptr from,
                                         direction const dir) {
  return filter_transform_collect(section.from(from, dir));
}

element::ids get_exclusion_elements_to(section const& section, element::ptr to,
                                       direction const dir) {
  return filter_transform_collect(section.to(to, dir));
}

element::ids get_exclusion_elements_from_to(section const& section,
                                            element::ptr from, element::ptr to,
                                            direction const dir) {
  return filter_transform_collect(section.from_to(from, to, dir));
}

section::ids get_used_sections(interlocking_route const& ir,
                               infrastructure const& infra) {
  section::ids used_sections;
  for (auto const& rn : ir.iterate(infra)) {
    auto const& element = rn.node_->element_;
    if (!element->is_track_element()) {
      continue;
    }

    auto const& sec_ids =
        infra->graph_.element_id_to_section_ids_[element->id()];
    utls::sassert(sec_ids.size() == 1,
                  "track element with more than one section?");
    auto const section_id = sec_ids.front();

    if (used_sections.empty() || used_sections.back() != section_id) {
      used_sections.emplace_back(section_id);
    }
  }

  return used_sections;
}

element::ids get_exclusion_elements_one_section(interlocking_route const& ir,
                                                section const& section,
                                                infrastructure const& infra) {
  element::ids exclusion_elements;

  auto const& first_element = ir.first_node(infra)->element_;
  auto const& last_element = ir.last_node(infra)->element_;

  direction dir = direction::Rising;

  if (first_element->is_track_element()) {
    dir = static_cast<direction>(first_element->as<track_element>().rising_);
  } else if (last_element->is_track_element()) {
    dir = static_cast<direction>(last_element->as<track_element>().rising_);
  }

  utl::concat(exclusion_elements,
              get_exclusion_elements_from_to(section, first_element,
                                             last_element, dir));
  utl::erase_duplicates(exclusion_elements);

  return exclusion_elements;
}

element::ids get_exclusion_elements(interlocking_route const& ir,
                                    infrastructure const& infra) {
  element::ids exclusion_elements;

  // second: gather all elements in the used sections

  auto const used_sections = get_used_sections(ir, infra);
  utls::sassert(!used_sections.empty(), "no used sections found");

  auto const& first_element = ir.first_node(infra)->element_;
  auto const& last_element = ir.last_node(infra)->element_;

  auto const& first_section = infra->graph_.sections_[used_sections.front()];
  auto const& last_section = infra->graph_.sections_[used_sections.back()];

  if (used_sections.size() == 1) {
    auto const& only_section = infra->graph_.sections_[used_sections.front()];
    return get_exclusion_elements_one_section(ir, only_section, infra);
  }

  if (!ir.starts_on_section(infra)) {
    auto const starts_rising =
        static_cast<direction>(first_element->as<track_element>().rising_);

    utl::concat(exclusion_elements,
                get_exclusion_elements_from(first_section, first_element,
                                            starts_rising));
  } else {
    utl::concat(exclusion_elements, get_exclusion_elements(first_section));
  }

  // the middle sections are all traversed in their entirety
  for (auto i = 1U; i < used_sections.size() - 1; ++i) {
    auto const& section = infra->graph_.sections_[used_sections[i]];
    utl::concat(exclusion_elements, get_exclusion_elements(section));
  }

  if (!ir.ends_on_section(infra)) {
    // the last section is not traversed in total, only gather one part
    auto const ends_rising =
        static_cast<direction>(last_element->as<track_element>().rising_);

    utl::concat(
        exclusion_elements,
        get_exclusion_elements_to(last_section, last_element, ends_rising));
  } else {
    utl::concat(exclusion_elements, get_exclusion_elements(last_section));
  }

  utl::erase_duplicates(exclusion_elements);

  return exclusion_elements;
}

soro::vector<element::ids> get_closed_exclusion_elements(
    infrastructure const& infra) {
  return soro::to_vec(infra->interlocking_.routes_, [&](auto&& ir) {
    return get_exclusion_elements(ir, infra);
  });
}

soro::vector<element::ids> get_open_exclusion_elements(
    soro::vector<element::ids> const& closed_exclusion_elements,
    infrastructure const& infra) {
  auto open_exclusion_elements = closed_exclusion_elements;

  for (auto const& ir : infra->interlocking_.routes_) {
    auto const first_element = ir.first_node(infra)->element_;
    auto const last_element = ir.last_node(infra)->element_;

    utl::erase(open_exclusion_elements[ir.id_], first_element->id());
    utl::erase(open_exclusion_elements[ir.id_], last_element->id());
  }

  return open_exclusion_elements;
}

}  // namespace soro::infra
