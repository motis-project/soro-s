#pragma once

namespace soro::utls {

template <std::size_t I, typename T, typename... Ts>
struct type_at_position : type_at_position<I - 1, Ts...> {};

template <typename T, typename... Ts>
struct type_at_position<0, T, Ts...> {
  using type = T;
};

template <std::size_t I, typename... Ts>
using type_at_position_t = typename type_at_position<I, Ts...>::type;

template <typename... Ts>
using first_t = typename type_at_position<0, Ts...>::type;

}  // namespace soro::utls