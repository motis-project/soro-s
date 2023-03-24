#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Pred>
constexpr std::size_t count_if(Iterable&& i, Pred&& p) {
  return static_cast<std::size_t>(std::count_if(begin(i), end(i), p));
}

}  // namespace detail

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr std::size_t count_if(Iterable&& i, Pred&& p) {
  return detail::count_if(std::forward<Iterable>(i), std::forward<Pred>(p));
}

}  // namespace soro::utls
