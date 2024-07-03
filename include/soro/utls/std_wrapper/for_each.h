#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename Unary>
constexpr void for_each(Iterable&& i, Unary&& u) {
  std::for_each(begin(std::forward<Iterable>(i)),
                end(std::forward<Iterable>(i)), std::forward<Unary>(u));
}

}  // namespace detail

template <typename Iterable, typename Unary>
constexpr void for_each(Iterable&& i, Unary&& u) {
  detail::for_each(std::forward<Iterable>(i), std::forward<Unary>(u));
}

}  // namespace soro::utls
