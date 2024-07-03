#pragma once

#include <iterator>
#include <numeric>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable>
constexpr auto min_element(Iterable const& i) {
  return std::min_element(begin(i), end(i));
}

template <typename Iterable, typename Pred>
constexpr auto min_element(Iterable const& i, Pred&& p) {
  return std::min_element(begin(i), end(i), std::forward<Pred>(p));
}

}  // namespace detail

template <typename Iterable>
[[nodiscard]] constexpr auto min_element(Iterable&& i) {
  return detail::min_element(std::forward<Iterable>(i));
}

template <typename Iterable, typename Pred>
[[nodiscard]] constexpr auto min_element(Iterable&& i, Pred&& p) {
  return detail::min_element(std::forward<Iterable>(i), std::forward<Pred>(p));
}

}  // namespace soro::utls
