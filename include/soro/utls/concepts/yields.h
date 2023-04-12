#pragma once

#include <iterator>
#include <utility>

#include "soro/utls/concepts/is_any_of.h"

namespace soro::utls {

namespace detail {

using std::begin;
using std::cbegin;

#if (__clang_major__ >= 16)
static_assert(false, "remove the format workaround when clang 16 is released");
#endif

// clang-format off

// using begin(std::as_const(i)) instead of cbegin(i) because
// std::filesystem::cbegin(directory_iterator) does not exist
template <typename R, typename Iterable>
concept yields_impl = requires(Iterable&& i) {
  { *begin(i) } -> is_any_of<R, R&, R const, R const&>;
  { *begin(std::as_const(i)) } -> is_any_of<R, R const, R const&>;
};

template <typename R, typename Iterable, typename Pred>
concept predicate_yields_impl = requires(Iterable&& i, Pred&& p) {
  { p(*begin(i)) } -> is_any_of<R, R&, R const, R const&>;
  { p(*begin(std::as_const(i))) } -> is_any_of<R, R const, R const&>;
};

// clang-format on

}  // namespace detail

template <typename R, typename Iterable>
concept yields = detail::yields_impl<R, Iterable>;

template <typename R, typename Iterable, typename Pred>
concept predicate_yields = detail::predicate_yields_impl<R, Iterable, Pred>;

}  // namespace soro::utls
