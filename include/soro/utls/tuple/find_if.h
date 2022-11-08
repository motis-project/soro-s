#pragma once

#include <type_traits>

#include "soro/utls/tuple/is_tuple.h"

namespace soro::utls::tuple {

// --- find_if runtime for tuple --- //

template <typename Tuple, typename Predicate, std::size_t I,
          std::enable_if_t<is_tuple_v<Tuple>, bool> = true,
          std::enable_if_t<I == soro::tuple_size_v<Tuple>, bool> = true>
constexpr std::size_t find_if(Tuple const&, Predicate) {
  return soro::tuple_size_v<Tuple>;
}

template <typename Tuple, typename Predicate, std::size_t I = 0,
          std::enable_if_t<is_tuple_v<Tuple>, bool> = true,
          std::enable_if_t<I != soro::tuple_size_v<Tuple>, bool> = true>
constexpr std::size_t find_if(Tuple const& tuple, Predicate pred) {
  using std::get;
  if (pred(get<I>(tuple))) {
    return I;
  } else {
    return find_if<Tuple, Predicate, I + 1>(tuple, pred);
  }
}

}  // namespace soro::utls::tuple
