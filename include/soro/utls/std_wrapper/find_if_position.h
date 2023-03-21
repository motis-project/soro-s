#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Pred>
constexpr std::size_t find_if_position(Iterable&& i, Pred&& pred) {
  return static_cast<std::size_t>(
      std::distance(begin(i), find_if(begin(i), end(i), pred)));
}

}  // namespace detail

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr std::size_t find_if_position(Iterable&& i,
                                                     Pred&& pred) {
  return detail::find_if_position(std::forward<Iterable>(i),
                                  std::forward<Pred>(pred));
}

}  // namespace soro::utls
