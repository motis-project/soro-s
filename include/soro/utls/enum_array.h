#pragma once

#include "soro/utls/algo/fill_in.h"

namespace soro::utls {

template <typename Enum>
constexpr auto array_from_enum() {
  auto constexpr enum_size = static_cast<std::size_t>(Enum::invalid);

  static_assert(enum_size != 0);

  std::array<Enum, enum_size> result;
  fill_in(result, [](auto&& i) { return static_cast<Enum>(i); });

  return result;
}

}  // namespace soro::utls
