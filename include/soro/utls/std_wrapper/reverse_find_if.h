
#pragma once

#include <algorithm>
#include <iterator>

namespace soro::utls {

namespace detail {

using std::rbegin;
using std::rend;

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr auto reverse_find_if(Iterable const& i, Pred&& pred) {
  return std::find_if(std::rbegin(i), std::rend(i), std::forward<Pred>(pred));
}

}  // namespace detail

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr auto reverse_find_if(Iterable&& i, Pred&& pred) {
  return detail::reverse_find_if(std::forward<Iterable>(i),
                                 std::forward<Pred>(pred));
}

}  // namespace soro::utls
