#pragma once

#include <limits>

#include "soro/base/soro_types.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

// super sections connect the following elements:
// - bumpers
// - track ends
// - switches
// - crosses

struct super_section {
  using id = uint32_t;

  static constexpr id INVALID = std::numeric_limits<id>::max();

  element_id start_{element::INVALID};
  element_id end_{element::INVALID};
  soro::vector<section::id> sections_;
};

struct super_sections {
  soro::vector<super_section> super_sections_;

  // section::id -> super_section::id
  soro::vector<super_section::id> section_to_super_section_;

  // infra::element::id -> [super_section::id]
  // layout for the element types:
  //  - bumpers: [id]
  //  - track_ends: [id]
  //  - switches: [start id, stem id, branch id]
  //  - crosses: [start left id, start right id, end left id, end right id]
  //  - rest: [id]
  soro::vecvec<soro::size_t, super_section::id> element_to_super_sections_;
};

super_sections get_super_sections(graph const& graph);

}  // namespace soro::infra