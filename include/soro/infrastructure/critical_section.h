#pragma once

#include <limits>

#include "soro/base/soro_types.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

// critical sections connect the following elements:
// - bumpers
// - track ends
// - switches
// - crosses

struct critical_section {
  using id = uint32_t;

  static constexpr id INVALID = std::numeric_limits<id>::max();

  element_id start_{element::INVALID};
  element_id end_{element::INVALID};
  soro::vector<section::id> sections_;
};

struct critical_sections {
  soro::vector<critical_section> critical_sections_;

  // section::id -> critical_section::id
  soro::vector<critical_section::id> section_to_critical_section_;

  // infra::element::id -> [critical_section::id]
  // layout for the element types:
  //  - bumpers: [id]
  //  - track_ends: [id]
  //  - switches: [start id, stem id, branch id]
  //  - crosses: [start left id, start right id, end left id, end right id]
  //  - rest: [id]
  soro::vecvec<soro::size_t, critical_section::id>
      element_to_critical_sections_;
};

critical_sections get_critical_sections(graph const& graph);

}  // namespace soro::infra