#pragma once

#include <tuple>
#include <type_traits>

#include "soro/utls/tuple/is_tuple.h"

namespace soro::utls::tuple {

// --- for_each runtime for tuple --- //

template <typename Tuple, typename Fun, std::size_t I,
          std::enable_if_t<is_tuple_v<Tuple>, bool> = true,
          std::enable_if_t<I == std::tuple_size_v<Tuple>, bool> = true>
constexpr void for_each(Tuple&, Fun) {}

template <typename Tuple, typename Fun, std::size_t I = 0,
          std::enable_if_t<is_tuple_v<Tuple>, bool> = true,
          std::enable_if_t<I != std::tuple_size_v<Tuple>, bool> = true>
constexpr void for_each(Tuple& tuple, Fun f) {
  using std::get;
  f(get<I>(tuple));
  for_each<Tuple, Fun, I + 1>(tuple, f);
}

template <typename Tuple, typename Fun, std::size_t I,
          std::enable_if_t<is_tuple_v<Tuple>, bool> = true,
          std::enable_if_t<I == std::tuple_size_v<Tuple>, bool> = true>
constexpr void for_each(Tuple const&, Fun) {}

template <typename Tuple, typename Fun, std::size_t I = 0,
          std::enable_if_t<is_tuple_v<Tuple>, bool> = true,
          std::enable_if_t<I != std::tuple_size_v<Tuple>, bool> = true>
constexpr void for_each(Tuple const& tuple, Fun f) {
  using std::get;
  f(get<I>(tuple));
  for_each<Tuple, Fun, I + 1>(tuple, f);
}

}  // namespace soro::utls::tuple