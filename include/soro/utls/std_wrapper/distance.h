#pragma once

#include <algorithm>

#include "soro/utls/narrow.h"

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterator>
constexpr auto distance(Iterator&& from, Iterator&& to) {
  return std::distance(from, to);
}

template <typename Iterable>
constexpr auto distance(Iterable&& i) {
  return std::distance(begin(i), end(i));
}

}  // namespace detail

template <std::integral T, typename Iterator>
[[nodiscard]] constexpr T distance(Iterator const& from, Iterator const& to) {
  return utls::narrow<T>(detail::distance(from, to));
}

template <std::integral T, typename Iterable>
[[nodiscard]] constexpr T distance(Iterable&& i) {
  return utls::narrow<T>(detail::distance(i));
}

}  // namespace soro::utls
