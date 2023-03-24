#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable>
constexpr void sort(Iterable&& c) {
  std::sort(begin(c), end(c));
}

template <typename Iterable, typename Pred>
constexpr void sort(Iterable&& c, Pred&& pred) {
  std::sort(begin(c), end(c), pred);
}

}  // namespace detail

template <typename Iterable>
constexpr void sort(Iterable&& i) {
  detail::sort(std::forward<Iterable>(i));
}

template <typename Iterable, typename Pred>
constexpr void sort(Iterable&& i, Pred&& pred) {
  detail::sort(std::forward<Iterable>(i), std::forward<Pred>(pred));
}

}  // namespace soro::utls
