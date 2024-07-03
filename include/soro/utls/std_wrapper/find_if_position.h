#pragma once

#include <algorithm>

#include "soro/utls/narrow.h"

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Pred>
constexpr auto find_if_position(Iterable&& i, Pred&& pred) {
  return std::distance(begin(i), std::find_if(begin(i), end(i), pred));
}

template <typename FromIter, typename ToIter, typename Pred>
constexpr auto find_if_position(FromIter&& from, ToIter&& to, Pred&& pred) {
  return std::distance(from, std::find_if(from, to, pred));
}

}  // namespace detail

template <std::integral Out, typename Iterable, typename Pred>
[[nodiscard]] constexpr Out find_if_position(Iterable&& i, Pred&& pred) {
  return utls::narrow<Out>(detail::find_if_position(std::forward<Iterable>(i),
                                                    std::forward<Pred>(pred)));
}

template <std::integral Out, typename FromIter, typename ToIter, typename Pred>
[[nodiscard]] constexpr Out find_if_position(FromIter&& from, ToIter&& to,
                                             Pred&& pred) {
  return utls::narrow<Out>(detail::find_if_position(
      std::forward<FromIter>(from), std::forward<ToIter>(to),
      std::forward<Pred>(pred)));
}

}  // namespace soro::utls
