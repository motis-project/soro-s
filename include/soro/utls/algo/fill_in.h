#pragma once

#include <algorithm>

#include "utl/enumerate.h"

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename UnaryGenerator>
constexpr void fill_in(Iterable&& i, UnaryGenerator&& p) {
  for (auto [idx, v] : utl::enumerate(std::forward<Iterable>(i))) {
    v = p(idx);
  }
}

}  // namespace detail

template <typename Iterable, typename UnaryGenerator>
constexpr void fill_in(Iterable&& i, UnaryGenerator&& t) {
  return detail::fill_in(std::forward<Iterable>(i),
                         std::forward<UnaryGenerator>(t));
}

}  // namespace soro::utls
