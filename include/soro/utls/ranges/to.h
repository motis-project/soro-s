#pragma once

#include <ranges>

namespace soro::utls {

namespace detail {

template <std::ranges::range R>
using range_value_t = decltype(*std::begin(std::declval<R>()));

template <template <typename...> class ContT>
struct templated_container {
  template <typename T>
  using with_value_t = ContT<std::remove_const_t<std::remove_reference_t<T>>>;
};

template <typename R, typename TemplatedContainer>
auto operator|(R&& r, TemplatedContainer) ->
    typename TemplatedContainer::template with_value_t<range_value_t<R>> {
  using std::begin, std::end;
  return {begin(r), end(r)};
}

}  // namespace detail

template <template <typename...> class Container>
auto to() -> detail::templated_container<Container> {
  return {};
}

}  // namespace soro::utls