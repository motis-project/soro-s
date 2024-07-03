#pragma once

#include <type_traits>

namespace soro::utls {

template <auto From, auto To, auto Inc, typename F>
constexpr void constexpr_for(F&& f) {
  if constexpr (From < To) {
    f(std::integral_constant<decltype(From), From>{});
    constexpr_for<From + Inc, To, Inc>(f);
  }
}

}  // namespace soro::utls