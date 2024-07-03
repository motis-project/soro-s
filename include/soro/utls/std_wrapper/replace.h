#pragma once

#include <algorithm>
#include <iterator>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr void replace(Iterable&& i, T&& old_value, T&& new_value) {
  std::replace(begin(i), end(i), old_value, new_value);
}

}  // namespace detail

template <typename Iterable, typename T>
constexpr void replace(Iterable&& i, T&& old_value, T&& new_value) {
  return detail::replace(std::forward<Iterable>(i), std::forward<T>(old_value),
                         std::forward<T>(new_value));
}

}  // namespace soro::utls
