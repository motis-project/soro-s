#pragma once

#include <algorithm>

#include "soro/utls/narrow.h"

#include "soro/base/soro_types.h"

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
[[nodiscard]] constexpr soro::size_t count_if(Iterable&& i, Pred&& p) {
  return narrow<soro::size_t>(
      detail::count_if(std::forward<Iterable>(i), std::forward<Pred>(p)));
}

}  // namespace soro::utls
