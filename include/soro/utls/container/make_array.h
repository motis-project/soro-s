#pragma once

#include <algorithm>
#include <array>

namespace soro::utls {

template <typename T, std::size_t Size>
constexpr auto make_array(T const& init) {
  std::array<T, Size> result{};
  std::fill(std::begin(result), std::end(result), init);
  return result;
}

}  // namespace soro::utls
