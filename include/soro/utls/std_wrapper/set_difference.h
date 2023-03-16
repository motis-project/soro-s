#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable1, typename Iterable2, typename OutputIt>
OutputIt constexpr set_difference(Iterable1&& i1, Iterable2&& i2,
                                  OutputIt&& out) {
  return std::set_difference(begin(i1), end(i1), begin(i2), end(i2), out);
}

}  // namespace detail

template <typename Iterable1, typename Iterable2, typename OutputIt>
OutputIt constexpr set_difference(Iterable1&& i1, Iterable2&& i2,
                                  OutputIt&& out) {
  return detail::set_difference(std::forward<Iterable1>(i1),
                                std::forward<Iterable2>(i2),
                                std::forward<OutputIt>(out));
}

}  // namespace soro::utls
