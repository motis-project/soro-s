#pragma once

#include "soro/base/soro_types.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

// super sections only connect the following elements:
// - bumpers
// - track ends
// - switches
// - crosses

struct super_section {
  using id = uint32_t;

  element_id start_;
  element_id end_;
};

struct super_sections {
  std::vector<super_section> super_sections_;

  // section::id -> super_section::id
  std::vector<super_section::id> section_to_super_section_;

  // infra::element::id -> [super_section::id]
  // layout for the element types:
  //  - bumpers: [id]
  //  - track_ends: [id]
  //  - switches: [start id, stem id, branch id]
  //  - crosses: [start left id, start right id, end left id, end right id]
  soro::vecvec<soro::size_t, super_section::id> element_to_super_sections_;
};

inline super_sections get_super_sections(soro::vector<section> const&) {
  utls::sassert(false, "not implemented");
  return {};
}

}  // namespace soro::infra