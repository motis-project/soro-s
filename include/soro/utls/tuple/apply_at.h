#pragma once

#include <type_traits>

#include "utl/verify.h"

namespace soro::utls::tuple {

// --- apply_at runtime for tuple --- //
// applies the given function on the element with the specified idx

template <typename Tuple, typename Fun, std::size_t I,
          std::enable_if_t<I == soro::tuple_size_v<Tuple>, bool> = true>
constexpr void apply_at(Tuple&, std::size_t const idx, Fun) {
  throw utl::fail(
      "In function apply_at in {}: The specified index was {}, but the tuple "
      "only has {} elements.",
      __FILE__, idx, soro::tuple_size_v<Tuple>);
}

template <typename Tuple, typename Fun, std::size_t I = 0,
          std::enable_if_t<I != soro::tuple_size_v<Tuple>, bool> = true>
constexpr void apply_at(Tuple& tuple, std::size_t const idx, Fun f) {
  using std::get;
  if (I == idx) {
    f(get<I>(tuple));
  } else {
    apply_at<Tuple, Fun, I + 1>(tuple, idx, f);
  }
}

template <typename Tuple, typename Fun, std::size_t I,
          std::enable_if_t<I == soro::tuple_size_v<Tuple>, bool> = true>
constexpr void apply_at(Tuple const&, std::size_t const idx, Fun) {
  throw utl::fail(
      "In function apply_at in {}: The specified index was {}, but the tuple "
      "only has {} elements.",
      __FILE__, idx, soro::tuple_size_v<Tuple>);
}

template <typename Tuple, typename Fun, std::size_t I = 0,
          std::enable_if_t<I != soro::tuple_size_v<Tuple>, bool> = true>
constexpr void apply_at(Tuple const& tuple, std::size_t const idx, Fun f) {
  using std::get;
  if (I == idx) {
    f(get<I>(tuple));
  } else {
    apply_at<Tuple, Fun, I + 1>(tuple, idx, f);
  }
}

}  // namespace soro::utls::tuple
