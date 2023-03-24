#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr auto find(Iterable&& i, T&& v) {
  return std::find(begin(i), end(i), v);
}

}  // namespace detail

template <typename Iterable, typename T>
[[nodiscard]] constexpr auto find(Iterable&& i, T&& v) {
  return detail::find(std::forward<Iterable>(i), std::forward<T>(v));
}

}  // namespace soro::utls
