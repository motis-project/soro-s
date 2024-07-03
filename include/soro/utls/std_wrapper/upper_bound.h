#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr auto upper_bound(Iterable&& i, T&& value) {
  return std::upper_bound(begin(i), end(i), std::forward<T>(value));
}

template <typename Iterable, typename T, typename Comp>
constexpr auto upper_bound(Iterable&& i, T&& value, Comp&& comp) {
  return std::upper_bound(begin(i), end(i), std::forward<T>(value),
                          std::forward<Comp>(comp));
}

}  // namespace detail

template <typename Iterable, typename T>
[[nodiscard]] constexpr auto upper_bound(Iterable&& i, T&& value) {
  return detail::upper_bound(std::forward<Iterable>(i), std::forward<T>(value));
}

template <typename Iterable, typename T, typename Comp>
[[nodiscard]] constexpr auto upper_bound(Iterable&& i, T&& value, Comp&& comp) {
  return detail::upper_bound(std::forward<Iterable>(i), std::forward<T>(value),
                             std::forward<Comp>(comp));
}

}  // namespace soro::utls
