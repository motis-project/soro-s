#pragma once

#include <algorithm>
#include <vector>

#include "soro/utls/concepts/yields.h"

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Pred>
constexpr bool any_of(Iterable const& i, Pred&& p) {
  return std::any_of(begin(i), end(i), std::forward<Pred>(p));
}

template <typename Iterable>
  requires yields<bool, Iterable> ||
           std::same_as<std::decay_t<Iterable>, std::vector<bool>>
constexpr bool any_of(Iterable const& i) {
  return std::any_of(begin(i), end(i), [](auto&& v) { return v; });
}

}  // namespace detail

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr bool any_of(Iterable&& i, Pred&& p) {
  return detail::any_of(std::forward<Iterable>(i), std::forward<Pred>(p));
}

template <typename Iterable>
[[nodiscard]] constexpr bool any_of(Iterable&& i) {
  return detail::any_of(std::forward<Iterable>(i));
}

}  // namespace soro::utls
