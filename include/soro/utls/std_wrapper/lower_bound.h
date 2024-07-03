#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr auto lower_bound(Iterable&& i, T&& value) {
  return std::lower_bound(begin(i), end(i), std::forward<T>(value));
}

template <typename Iterable, typename T, typename Comp>
constexpr auto lower_bound(Iterable&& i, T&& value, Comp&& comp) {
  return std::lower_bound(begin(i), end(i), std::forward<T>(value),
                          std::forward<Comp>(comp));
}

}  // namespace detail

template <typename Iterable, typename T>
[[nodiscard]] constexpr auto lower_bound(Iterable&& i, T&& value) {
  return detail::lower_bound(std::forward<Iterable>(i), std::forward<T>(value));
}

template <typename Iterable, typename T, typename Comp>
[[nodiscard]] constexpr auto lower_bound(Iterable&& i, T&& value, Comp&& comp) {
  return detail::lower_bound(std::forward<Iterable>(i), std::forward<T>(value),
                             std::forward<Comp>(comp));
}

}  // namespace soro::utls
