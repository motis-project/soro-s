#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Iterable, typename OutputIt, typename Unary>
constexpr void transform(Iterable&& i, OutputIt&& out, Unary&& u) {
  std::transform(begin(std::forward<Iterable>(i)),
                 end(std::forward<Iterable>(i)), std::forward<OutputIt>(out),
                 std::forward<Unary>(u));
}

}  // namespace detail

template <typename Iterable, typename OutputIt, typename Unary>
constexpr void transform(Iterable&& i, OutputIt&& out, Unary&& u) {
  detail::transform(std::forward<Iterable>(i), std::forward<OutputIt>(out),
                    std::forward<Unary>(u));
}

}  // namespace soro::utls
