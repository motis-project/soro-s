#pragma once

#include <iterator>

#include "soro/utls/container/it_range.h"

namespace soro::utls {

namespace detail {

using std::rbegin;
using std::rend;

template <typename Iterable>
constexpr auto reverse_range(Iterable&& iterable) {
  return utls::it_range(rbegin(iterable), rend(iterable));
}

}  // namespace detail

template <typename Iterable>
[[nodiscard]] constexpr auto reverse_range(Iterable&& iterable) {
  return detail::reverse_range(std::forward<Iterable>(iterable));
}

}  // namespace soro::utls