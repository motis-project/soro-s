#pragma once

#include <algorithm>

#include "soro/utls/concepts/yields.h"

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Pred>
  requires utls::predicate_yields<bool, Iterable, Pred>
constexpr auto find_if(Iterable&& i, Pred&& pred) {
  return std::find_if(begin(i), end(i), pred);
}

}  // namespace detail

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr auto find_if(Iterable&& i, Pred&& pred) {
  return detail::find_if(std::forward<Iterable>(i), std::forward<Pred>(pred));
}

}  // namespace soro::utls
