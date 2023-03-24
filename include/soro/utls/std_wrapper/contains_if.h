#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Container, typename Pred>
constexpr bool contains_if(Container&& c, Pred&& pred) {
  return std::find_if(begin(c), end(c), pred) != end(c);
}

}  // namespace detail

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr bool contains_if(Iterable&& i, Pred&& p) {
  return detail::contains_if(std::forward<Iterable>(i), std::forward<Pred>(p));
}

}  // namespace soro::utls
