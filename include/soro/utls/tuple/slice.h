#pragma once

#include <tuple>
#include <type_traits>

namespace soro::utls::tuple {

namespace detail {

template <std::size_t Int, std::size_t... Ints>
constexpr auto remove_first(std::integer_sequence<std::size_t, Int, Ints...>) {
  return std::integer_sequence<std::size_t, Ints...>{};
}

template <typename T, T From, T To, T... Ints>
constexpr auto get_integer_sequence(std::integer_sequence<T, Ints...> ints) {
  if constexpr (sizeof...(Ints) != To - From) {
    return get_integer_sequence<T, From, To>(remove_first<Ints...>(ints));
  } else {
    return ints;
  }
}

template <typename T, T From, T To>
constexpr auto make_integer_sequence() {
  static_assert(From <= To);
  return get_integer_sequence<T, From, To>(std::make_index_sequence<To>{});
}

template <typename... Ts, std::size_t... Ints>
constexpr auto slice(std::tuple<Ts...> const& t,
                     std::integer_sequence<std::size_t, Ints...>) {
  return std::make_tuple(std::get<Ints>(t)...);
}

}  // namespace detail

template <std::size_t From, std::size_t To, typename... Ts>
constexpr auto slice(std::tuple<Ts...> const& t) {
  return detail::slice(t,
                       detail::make_integer_sequence<std::size_t, From, To>());
}

}  // namespace soro::utls::tuple
