#pragma once

#include <algorithm>

#include "soro/utls/concepts/yields.h"

namespace soro::utls {

namespace detail {

using std::rbegin;
using std::rend;

template <typename Iterable, typename Pred>
  requires utls::predicate_yields<bool, Iterable, Pred>
constexpr auto find_if_reverse(Iterable&& i, Pred&& pred) {
  return std::find_if(rbegin(i), rend(i), pred);
}

}  // namespace detail

template <typename Iterable, typename Pred>
  requires utls::predicate_yields<bool, Iterable, Pred>
[[nodiscard]] constexpr auto find_if_reverse(Iterable&& i, Pred&& pred) {
  return detail::find_if_reverse(std::forward<Iterable>(i),
                                 std::forward<Pred>(pred));
}

}  // namespace soro::utls
