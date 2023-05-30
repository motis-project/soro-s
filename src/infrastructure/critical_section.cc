#include "soro/infrastructure/critical_section.h"

#include "utl/enumerate.h"
#include "utl/timer.h"

#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/contains_if.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/type_set.h"

namespace soro::infra {

template <typename T, std::size_t Size>
constexpr auto make_array(T const& init) {
  std::array<T, Size> result{};
  std::fill(std::begin(result), std::end(result), init);
  return result;
}

auto initialize_element_to_critical_sections(graph const& graph) {
  soro::vecvec<soro::size_t, critical_section::id> result;

  for (auto const& e : graph.elements_) {
    switch (e->type()) {
      case type::SIMPLE_SWITCH: {
        result.emplace_back(
            make_array<critical_section::id, 3>(critical_section::INVALID));
        break;
      }

      case type::CROSS: {
        result.emplace_back(
            make_array<critical_section::id, 4>(critical_section::INVALID));
        break;
      }

      case type::TRACK_END:
      case type::BUMPER:
      default: {
        result.emplace_back(
            make_array<critical_section::id, 1>(critical_section::INVALID));
        break;
      }
    }
  }

  return result;
}

static const type_set critical_section_elements = {
    type::SIMPLE_SWITCH, type::CROSS, type::TRACK_END, type::BUMPER};

critical_section get_critical_section(section::id section_id,
                                      graph const& graph,
                                      element::ptr const& from) {
  // section_id is not const, we are going to use it as a variable

  utls::expects([&] {
    auto const& section = graph.sections_[section_id];
    utls::expect(
        critical_section_elements.contains(section.first_rising()->type()) ||
            critical_section_elements.contains(section.last_rising()->type()),
        "section_id does not start a super section");
  });

  critical_section result;

  result.start_ = from->id();
  result.sections_ = {section_id};

  auto const get_next_section = [&graph](element::ptr const& to,
                                         section::id const current) {
    utls::sassert(to->is_simple_element());

    auto const& section_ids = graph.element_id_to_section_ids_[to->id()];
    utls::sassert(section_ids.size() == 2, "simple element needs 2 sections");

    return section_ids.front() == current ? section_ids.back()
                                          : section_ids.front();
  };

  auto to = graph.sections_[section_id].opposite_end(from);

  do {
    if (to->is_simple_element()) {
      section_id = get_next_section(to, section_id);
      to = graph.sections_[section_id].opposite_end(to);
      result.sections_.emplace_back(section_id);
    }
  } while (!critical_section_elements.contains(to->type()));

  result.end_ = to->id();

  return result;
}

utls::optional<element::ptr> get_critical_section_start(
    section const& section) {
  if (critical_section_elements.contains(section.first_rising()->type())) {
    return utls::optional<element::ptr>{section.first_rising()};
  }

  if (critical_section_elements.contains(section.first_falling()->type())) {
    return utls::optional<element::ptr>{section.first_falling()};
  }

  return std::nullopt;
}

soro::size_t get_bucket_idx_cross(cross const& cross, section const& section) {
  for (auto const& e : section.iterate<direction::Rising>()) {
    if (e->id() == cross.rising_start_left()->id() ||
        e->id() == cross.falling_start_left()->id()) {
      return 0;
    }

    if (e->id() == cross.rising_start_right()->id() ||
        e->id() == cross.falling_start_right()->id()) {
      return 1;
    }

    if (e->id() == cross.rising_end_left()->id() ||
        e->id() == cross.falling_end_left()->id()) {
      return 2;
    }

    if (e->id() == cross.rising_end_right()->id() ||
        e->id() == cross.falling_end_right()->id()) {
      return 3;
    }
  }

  utls::sassert(false, "cross not in section");

  std::unreachable();
}

soro::size_t get_bucket_idx_switch(simple_switch const& sswitch,
                                   section const& section) {

  for (auto const& e : section.iterate<direction::Rising>()) {
    if (e->id() == sswitch.rising_start_neighbour()->id() ||
        e->id() == sswitch.falling_start_neighbour()->id()) {
      return 0;
    }

    if (e->id() == sswitch.rising_stem_neighbour()->id() ||
        e->id() == sswitch.falling_stem_neighbour()->id()) {
      return 1;
    }

    if (e->id() == sswitch.rising_branch_neighbour()->id() ||
        e->id() == sswitch.falling_branch_neighbour()->id()) {
      return 2;
    }
  }

  utls::sassert(false, "switch not in section");
  std::unreachable();
}

soro::size_t get_bucket_idx(element::ptr const& element,
                            section const& section) {
  if (element->is(type::CROSS)) {
    return get_bucket_idx_cross(element->as<cross>(), section);
  }

  if (element->is(type::SIMPLE_SWITCH)) {
    return get_bucket_idx_switch(element->as<simple_switch>(), section);
  }

  return 0;
}

void set_element_to_critical_section_bucket(
    critical_section::id const critical_section_id,
    critical_section const& critical_section,
    soro::vecvec<soro::size_t, critical_section::id>&
        element_to_critical_sections,
    graph const& graph) {

  for (auto const section_id : critical_section.sections_) {
    auto const& section = graph.sections_[section_id];

    for (auto const& e : section.iterate<direction::Rising, skip::No>()) {
      utls::sasserts([&] {
        auto const bucket_idx = get_bucket_idx(e, section);
        auto const current_id =
            element_to_critical_sections[e->id()][bucket_idx];

        // no overwriting
        utls::sassert(current_id == critical_section::INVALID ||
                      current_id == critical_section_id);
      });

      element_to_critical_sections[e->id()][get_bucket_idx(e, section)] =
          critical_section_id;
    }
  }
}

critical_sections get_critical_sections(graph const& graph) {
  utl::scoped_timer const timer{"generating super sections"};

  critical_sections ss;

  ss.section_to_critical_section_.resize(graph.sections_.size(),
                                         critical_section::INVALID);
  ss.element_to_critical_sections_ =
      initialize_element_to_critical_sections(graph);

  // loop over the sections and every time we find a section that starts a
  // super section, we create a new super section and assign all the sections
  for (auto const& [section_id, section] : utl::enumerate(graph.sections_)) {
    // already assigned to a super section
    if (ss.section_to_critical_section_[section_id] != section::INVALID) {
      continue;
    }

    auto const ss_start = get_critical_section_start(section);

    // does not start a super section
    if (!ss_start) {
      continue;
    }

    // create new super section and assign all the sections
    auto critical_section = get_critical_section(section_id, graph, *ss_start);
    for (auto const& s_id : critical_section.sections_) {
      utls::sassert(
          ss.section_to_critical_section_[s_id] == critical_section::INVALID,
          "section already assigned to a super section");
      ss.section_to_critical_section_[s_id] = ss.critical_sections_.size();
    }
    set_element_to_critical_section_bucket(
        ss.critical_sections_.size(), critical_section,
        ss.element_to_critical_sections_, graph);
    ss.critical_sections_.emplace_back(std::move(critical_section));
  }

  utls::ensures([&ss, &graph] {
    utls::ensure(!ss.critical_sections_.empty());

    for (auto const& critical_section : ss.critical_sections_) {
      utls::ensure(critical_section.start_ != element::INVALID);
      utls::ensure(critical_section.end_ != element::INVALID);

      auto const& start_element = graph.elements_[critical_section.start_];
      utls::ensure(critical_section_elements.contains(start_element->type()));

      auto const& end_element = graph.elements_[critical_section.end_];
      utls::ensure(critical_section_elements.contains(end_element->type()));

      auto const& first_section =
          graph.sections_[critical_section.sections_.front()];
      utls::ensure(
          first_section.first_rising()->id() == critical_section.start_ ||
          first_section.last_rising()->id() == critical_section.start_);

      auto const& last_section =
          graph.sections_[critical_section.sections_.back()];
      utls::ensure(last_section.last_rising()->id() == critical_section.end_ ||
                   last_section.last_falling()->id() == critical_section.end_);

      auto const critical_section_element_count = utls::accumulate(
          critical_section.sections_, soro::size_t{0},
          [&](auto&& acc, auto&& section_id) {
            auto const& section = graph.sections_[section_id];

            auto const starts = critical_section_elements.contains(
                section.first_rising()->type());
            auto const ends = critical_section_elements.contains(
                section.last_rising()->type());

            return acc + static_cast<soro::size_t>(starts) +
                   static_cast<soro::size_t>(ends);
          });

      utls::ensure(critical_section_element_count == 2);
    }

    utls::ensure(ss.section_to_critical_section_.size() ==
                 graph.sections_.size());
    for (section::id s_id = 0; s_id < graph.sections_.size(); ++s_id) {
      auto const& section = graph.sections_[s_id];
      std::ignore = section;
      utls::ensure(ss.section_to_critical_section_[s_id] !=
                   critical_section::INVALID);
    }

    for (auto const& critical_sections : ss.element_to_critical_sections_) {
      for (auto const& ss_id : critical_sections) {
        utls::ensure(ss_id != critical_section::INVALID);
      }
    }
  });

  return ss;
}

};  // namespace soro::infra
