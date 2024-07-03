#pragma once

#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/template/first.h"
#include "soro/utls/template/is_initializer_list.h"

namespace soro::utls {

template <typename Iterable>
struct all {
  using value_type = decltype(*std::begin(std::declval<Iterable>()));

  constexpr explicit all(Iterable&& i) : i_{std::move(i)} {}
  constexpr explicit all(Iterable const& i) : i_{i} {}

  constexpr bool operator==(value_type const& v) const {
    return utls::all_of(i_, [&](auto&& i) { return i == v; });
  }

  std::conditional_t<std::movable<Iterable> && !is_initializer_list_v<Iterable>,
                     Iterable, Iterable const&>
      i_;
};

template <typename... Vs>
  requires(sizeof...(Vs) > 1)
all(Vs...) -> all<std::initializer_list<first_t<Vs...>>>;

}  // namespace soro::utls
