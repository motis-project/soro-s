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
  using id = soro::strong<uint32_t, struct _critical_section_id>;

  static constexpr id invalid() { return id::invalid(); }

  bool ok(graph const& graph) const;

  element::id start_{element::invalid()};
  element::id end_{element::invalid()};
  soro::vector<section::id> sections_;
};

struct critical_sections {
  bool ok(graph const& graph) const;

  soro::vector<critical_section> critical_sections_;

  soro::vector_map<section::id, critical_section::id>
      section_to_critical_section_;

  // element::id -> [critical_section::id]
  // layout for the element types:
  //  - bumpers: [id]
  //  - track_ends: [id]
  //  - switches: [start id, stem id, branch id]
  //  - crosses: [start left id, start right id, end left id, end right id]
  //  - rest: [id]
  soro::vecvec<element::id, critical_section::id> element_to_critical_sections_;
};

critical_sections get_critical_sections(graph const& graph);

}  // namespace soro::infra