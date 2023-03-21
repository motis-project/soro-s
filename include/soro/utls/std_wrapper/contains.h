#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename ValueType>
constexpr bool contains(Iterable&& c, ValueType&& e) {
  return std::find(begin(c), end(c), e) != end(c);
}

}  // namespace detail

template <typename Iterable, typename ValueType>
[[nodiscard]] constexpr bool contains(Iterable&& i, ValueType&& v) {
  return detail::contains(std::forward<Iterable>(i),
                          std::forward<ValueType>(v));
}

}  // namespace soro::utls