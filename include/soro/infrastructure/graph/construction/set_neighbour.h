#pragma once

#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

void set_neighbour(element& e, std::string const& name,
                   section::position const pos, element* neigh,
                   mileage_dir const dir);

namespace detail {

// helper accessor function to make assigning a neighbour more comfortable
template <typename Element, typename Direction>
void set_neighbour(Element&& e, mileage_dir const m_dir, Direction const dir,
                   element::ptr const neigh) {
  static_assert(
      std::is_same_v<Direction, typename std::decay_t<Element>::direction>);

  auto const neighbour_idx = detail::get_neighbour_idx(m_dir, dir);

  utls::sassert(neighbour_idx < e.neighbours_.size(), "idx not in range");
  utls::sassert(e.neighbours_[neighbour_idx] == nullptr, "no overwriting");

  e.neighbours_[neighbour_idx] = neigh;
}

}  // namespace detail

}  // namespace soro::infra