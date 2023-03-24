#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr std::size_t find_position(Iterable&& c, T&& v) {
  return static_cast<std::size_t>(
      std::distance(begin(c), std::find(begin(c), end(c), v)));
}

}  // namespace detail

template <typename Iterable, typename T>
[[nodiscard]] constexpr std::size_t find_position(Iterable&& i, T&& v) {
  return detail::find_position(std::forward<Iterable>(i), std::forward<T>(v));
}

}  // namespace soro::utls
