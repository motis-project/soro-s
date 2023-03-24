#pragma once

#include <iterator>
#include <numeric>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Pred>
constexpr auto min_element(Iterable&& i, Pred&& p) {
  return std::min_element(begin(i), end(i), p);
}

}  // namespace detail

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr auto min_element(Iterable&& i, Pred&& p) {
  return detail::min_element(std::forward<Iterable>(i), std::forward<Pred>(p));
}

}  // namespace soro::utls
