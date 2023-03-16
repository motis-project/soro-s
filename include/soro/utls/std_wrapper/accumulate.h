#pragma once

#include <iterator>
#include <numeric>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename T>
constexpr auto accumulate(Iterable&& i, T init) {
  return std::accumulate(begin(i), end(i), init);
}

template <typename Iterable, typename T, typename Fn>
constexpr auto accumulate(Iterable&& i, T init, Fn&& fn) {
  return std::accumulate(begin(i), end(i), init, fn);
}

}  // namespace detail

template <typename Iterable, typename T>
[[nodiscard]] constexpr auto accumulate(Iterable&& i, T init) {
  return detail::accumulate(std::forward<Iterable>(i), std::forward<T>(init));
}

template <typename Iterable, typename T, typename Fn>
[[nodiscard]] constexpr auto accumulate(Iterable&& i, T init, Fn&& fn) {
  return detail::accumulate(std::forward<Iterable>(i), std::forward<T>(init),
                            std::forward<Fn>(fn));
}

}  // namespace soro::utls
