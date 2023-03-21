#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename DestIt>
constexpr auto copy(Iterable&& i, DestIt&& dst) {
  return std::copy(begin(i), end(i), dst);
}

}  // namespace detail

template <typename Iterable, typename DestIt>
constexpr auto copy(Iterable&& i, DestIt&& dst) {
  return detail::copy(std::forward<Iterable>(i), std::forward<DestIt>(dst));
}

}  // namespace soro::utls
