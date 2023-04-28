#pragma once

#include <algorithm>
#include <iterator>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable>
constexpr void reverse(Iterable&& i) {
  std::reverse(begin(i), end(i));
}

}  // namespace detail

template <typename Iterable>
constexpr void reverse(Iterable&& i) {
  return detail::reverse(std::forward<Iterable>(i));
}

}  // namespace soro::utls
