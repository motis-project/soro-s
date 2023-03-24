#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr std::size_t count(Iterable&& i, T&& v) {
  return static_cast<std::size_t>(std::count(begin(i), end(i), v));
}

}  // namespace detail

template <typename Iterable, typename T>
[[nodiscard]] constexpr std::size_t count(Iterable&& i, T&& v) {
  return detail::count(std::forward<Iterable>(i), std::forward<T>(v));
}

}  // namespace soro::utls
