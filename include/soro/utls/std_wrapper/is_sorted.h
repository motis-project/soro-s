#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable>
constexpr bool is_sorted(Iterable&& i) {
  return std::is_sorted(begin(i), end(i));
}

template <typename Iterable, typename Comp>
constexpr bool is_sorted(Iterable&& i, Comp&& cmp) {
  return std::is_sorted(begin(i), end(i), cmp);
}

}  // namespace detail

template <typename Iterable>
[[nodiscard]] constexpr bool is_sorted(Iterable&& i) {
  return detail::is_sorted(std::forward<Iterable>(i));
}

template <typename Iterable, typename Comp>
[[nodiscard]] constexpr bool is_sorted(Iterable&& i, Comp&& cmp) {
  return detail::is_sorted(std::forward<Iterable>(i), std::forward<Comp>(cmp));
}

}  // namespace soro::utls
