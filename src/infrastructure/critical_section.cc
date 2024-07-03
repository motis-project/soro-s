#include "soro/infrastructure/critical_section.h"

#include <utility>

#include "utl/enumerate.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/container/make_array.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/all_of.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra {

using namespace utls;

soro::vecvec<element::id, critical_section::id>
allocate_element_to_critical_sections(graph const& graph) {
  soro::vecvec<element::id, critical_section::id> result;

  for (auto const& e : graph.elements_) {
    switch (e->type()) {
      case type::SIMPLE_SWITCH: {
        result.emplace_back(
            make_array<critical_section::id, 3>(critical_section::invalid()));
        break;
      }

      case type::CROSS: {
        result.emplace_back(
            make_array<critical_section::id, 4>(critical_section::invalid()));
        break;
      }

      case type::TRACK_END:
      case type::BUMPER:
      default: {
        result.emplace_back(
            make_array<critical_section::id, 1>(critical_section::invalid()));
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
        "section_id does not start a critical section");
  });

  critical_section result;

  result.start_ = from->get_id();
  result.sections_ = {section_id};

  auto const get_next_section = [&graph](element::ptr const& to,
                                         section::id const current) {
    utls::sassert(to->is_simple_element());

    auto const& section_ids =
        graph.sections_.element_to_section_ids_[to->get_id()];
    utls::sassert(section_ids.size() == 2, "simple element needs 2 sections");

    return section_ids.front() == current ? section_ids.back()
                                          : section_ids.front();
  };

  auto to = graph.sections_[section_id].opposite_end(from);

  do {  // NOLINT
    if (to->is_simple_element()) {
      section_id = get_next_section(to, section_id);
      to = graph.sections_[section_id].opposite_end(to);
      result.sections_.emplace_back(section_id);
    }
  } while (!critical_section_elements.contains(to->type()));

  result.end_ = to->get_id();

  return result;
}

soro::optional<element::ptr> get_critical_section_start(
    section const& section) {
  if (critical_section_elements.contains(section.first_rising()->type())) {
    return soro::optional<element::ptr>{section.first_rising()};
  }

  if (critical_section_elements.contains(section.first_falling()->type())) {
    return soro::optional<element::ptr>{section.first_falling()};
  }

  return {};
}

template <typename CrossOrSwitch>
soro::size_t get_bucket_idx(element::ptr const e, section const& section) {
  auto const& cos = e->as<CrossOrSwitch>();

  if (section.first_rising()->get_id() == cos.id_) {
    return static_cast<soro::size_t>(
        cos.get_neighbour_dir(section.second_rising()));
  }

  if (section.last_rising()->get_id() == cos.id_) {
    return static_cast<soro::size_t>(
        cos.get_neighbour_dir(section.second_to_last_rising()));
  }

  utls::sassert(false, "element not in section");

  std::unreachable();
}

soro::size_t get_bucket_idx(element::ptr const& element,
                            section const& section) {

  if (element->is(type::CROSS)) {
    return get_bucket_idx<cross>(element, section);
  }

  if (element->is(type::SIMPLE_SWITCH)) {
    return get_bucket_idx<simple_switch>(element, section);
  }

  return 0;
}

void set_element_to_critical_section_bucket(
    critical_section::id const critical_section_id,
    critical_section const& critical_section,
    soro::vecvec<element::id, critical_section::id>&
        element_to_critical_sections,
    graph const& graph) {

  for (auto const section_id : critical_section.sections_) {
    auto const& section = graph.sections_[section_id];

    for (auto const& e : section.iterate<mileage_dir::rising, skip::No>()) {
      utls::sasserts([&] {
        auto const bucket_idx = get_bucket_idx(e, section);
        auto const current_id =
            element_to_critical_sections[e->get_id()][bucket_idx];

        // no overwriting
        utls::sassert(current_id == critical_section::invalid() ||
                      current_id == critical_section_id);
      });

      element_to_critical_sections[e->get_id()][get_bucket_idx(e, section)] =
          critical_section_id;
    }
  }
}

bool critical_section::ok(graph const& graph) const {
  ENSURE(start_ != element::invalid(), "no valid start")
  ENSURE(end_ != element::invalid(), "no valid end")

  auto const& start_element = graph.elements_[start_];
  ENSURE(critical_section_elements.contains(start_element->type()),
         "invalid element type for start of critical section")

  auto const& end_element = graph.elements_[end_];
  ENSURE(critical_section_elements.contains(end_element->type()),
         "invalid element end type for end critical section")

  auto const& first_section = graph.sections_[sections_.front()];
  ENSURE(first_section.first_rising()->get_id() == start_ ||
             first_section.last_rising()->get_id() == start_,
         "first section does not start with critical section start")

  auto const& last_section = graph.sections_[sections_.back()];
  ENSURE(last_section.last_rising()->get_id() == end_ ||
             last_section.last_falling()->get_id() == end_,
         "last section does not end with critical section end")

  auto const critical_section_element_count = utls::accumulate(
      sections_, soro::size_t{0}, [&](auto&& acc, auto&& section_id) {
        auto const& section = graph.sections_[section_id];

        auto const starts =
            critical_section_elements.contains(section.first_rising()->type());
        auto const ends =
            critical_section_elements.contains(section.last_rising()->type());

        return acc + static_cast<soro::size_t>(starts) +
               static_cast<soro::size_t>(ends);
      });

  ENSURE(critical_section_element_count == 2,
         "every critical section must have exactly two element of "
         "critical section types")

  return true;
}

bool critical_sections::ok(graph const& graph) const {
  auto is_valid = [](auto&& id) { return id != critical_section::invalid(); };

  ENSURE(!critical_sections_.empty(), "critical sections must exist")

  ENSURE(all_of(critical_sections_, [&](auto&& cs) { return cs.ok(graph); }),
         "all critical sections must be ok")

  ENSURE(section_to_critical_section_.size() == graph.sections_.size(),
         "every section must be member of a critical section")

  ENSURE(all_of(section_to_critical_section_, is_valid),
         "every section must a member of a critical section")

  ENSURE(all_of(element_to_critical_sections_,
                [&](auto const cs) { return all_of(cs, is_valid); }),
         "no element can have an invalid critical section assigned")

  return true;
}

critical_sections get_critical_sections(graph const& graph) {
  utl::scoped_timer const timer{"generating critical sections"};

  critical_sections ss;

  ss.section_to_critical_section_.resize(graph.sections_.size(),
                                         critical_section::invalid());
  ss.element_to_critical_sections_ =
      allocate_element_to_critical_sections(graph);

  // loop over the sections and every time we find a section that starts
  // a critical section, we create a new critical section and assign all
  // the sections
  for (auto const [section_id, section] :
       utl::enumerate<section::id>(graph.sections_)) {
    // already assigned to a critical section
    if (ss.section_to_critical_section_[section_id] !=
        critical_section::invalid()) {
      continue;
    }

    auto const ss_start = get_critical_section_start(section);

    // does not start a critical section
    if (!ss_start) continue;

    // create new critical section and assign all the sections
    auto critical_section = get_critical_section(section_id, graph, *ss_start);
    for (auto const& s_id : critical_section.sections_) {
      utls::sassert(
          ss.section_to_critical_section_[s_id] == critical_section::invalid(),
          "section already assigned to a critical section");

      ss.section_to_critical_section_[s_id] =
          critical_section::id{ss.critical_sections_.size()};
    }

    set_element_to_critical_section_bucket(
        critical_section::id{ss.critical_sections_.size()}, critical_section,
        ss.element_to_critical_sections_, graph);
    ss.critical_sections_.emplace_back(std::move(critical_section));
  }

  utls::ensure(ss.ok(graph), "critical sections not ok");

  return ss;
}

};  // namespace soro::infra
