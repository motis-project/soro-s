#pragma once

#include <algorithm>

#include "soro/utls/narrow.h"

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable1, typename Iterable2>
constexpr auto equal(Iterable1 const& i1, Iterable2 const& i2) {
  return std::equal(begin(i1), end(i1), begin(i2));
}

}  // namespace detail

template <typename Iterable1, typename Iterable2>
  requires(!std::is_pointer_v<Iterable1> && !std::is_pointer_v<Iterable2>)
[[nodiscard]] constexpr bool equal(Iterable1&& i1, Iterable2&& i2) {
  return detail::equal(std::forward<Iterable1>(i1),
                       std::forward<Iterable2>(i2));
}

}  // namespace soro::utls
