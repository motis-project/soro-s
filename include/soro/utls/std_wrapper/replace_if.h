#pragma once

#include <algorithm>
#include <iterator>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Predicate, typename T>
constexpr void replace_if(Iterable&& i, Predicate&& unary, T&& new_value) {
  std::replace(begin(i), end(i), unary, new_value);
}

}  // namespace detail

template <typename Iterable, typename Predicate, typename T>
constexpr void replace(Iterable&& i, Predicate&& unary, T&& new_value) {
  return detail::replace_if(std::forward<Iterable>(i), std::forward<T>(unary),
                            std::forward<T>(new_value));
}

}  // namespace soro::utls
