#pragma once

#include "soro/utls/narrow.h"

#include "soro/infrastructure/kilometrage.h"

namespace soro::infra::detail {

// use the underlying type of mileage_dir as index into the element arrays
using element_array_idx = mileage_dir_t;

template <typename Direction>
constexpr element_array_idx get_neighbour_idx(mileage_dir const m_dir,
                                              Direction const dir) {
  auto const dir_product = std::to_underlying(dir) * 2;
  auto const m_dir_offset = std::to_underlying(m_dir);

  return utls::narrow<element_array_idx>(dir_product + m_dir_offset);
}

template <typename Node>
constexpr element_array_idx get_node_idx(Node const node) {
  return std::to_underlying(node);
}

template <typename Direction>
constexpr element_array_idx get_line_idx(Direction const dir) {
  return std::to_underlying(dir);
}

template <typename Direction>
constexpr element_array_idx get_km_idx(Direction const dir) {
  return std::to_underlying(dir);
}

}  // namespace soro::infra::detail
