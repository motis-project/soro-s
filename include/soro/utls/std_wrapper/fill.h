#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr void fill(Iterable&& i, T&& p) {
  return std::fill(begin(i), end(i), p);
}

}  // namespace detail

template <typename Iterable, typename T>
constexpr void fill(Iterable&& i, T&& t) {
  return detail::fill(std::forward<Iterable>(i), std::forward<T>(t));
}

}  // namespace soro::utls