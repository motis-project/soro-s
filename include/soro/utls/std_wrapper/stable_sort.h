#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable>
constexpr void stable_sort(Iterable&& c) {
  std::stable_sort(begin(c), end(c));
}

template <typename Iterable, typename Pred>
constexpr void stable_sort(Iterable&& c, Pred&& pred) {
  std::stable_sort(begin(c), end(c), pred);
}

}  // namespace detail

template <typename Iterable>
constexpr void stable_sort(Iterable&& i) {
  detail::stable_sort(std::forward<Iterable>(i));
}

template <typename Iterable, typename Pred>
constexpr void stable_sort(Iterable&& i, Pred&& pred) {
  detail::stable_sort(std::forward<Iterable>(i), std::forward<Pred>(pred));
}

}  // namespace soro::utls
