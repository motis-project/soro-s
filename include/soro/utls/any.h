#pragma once

#include "soro/utls/std_wrapper/any_of.h"
#include "soro/utls/template/first.h"
#include "soro/utls/template/is_initializer_list.h"

namespace soro::utls {

template <typename Iterable>
struct any {
  using value_type = decltype(*std::begin(std::declval<Iterable>()));

  constexpr explicit any(Iterable&& i) : i_{std::move(i)} {}
  constexpr explicit any(Iterable const& i) : i_{i} {}

  constexpr bool operator==(value_type const& v) const {
    return utls::any_of(i_, [&](auto&& i) { return i == v; });
  }

  std::conditional_t<std::movable<Iterable> && !is_initializer_list_v<Iterable>,
                     Iterable, Iterable const&>
      i_;
};

template <typename... Vs>
  requires(sizeof...(Vs) > 1)
any(Vs...) -> any<std::initializer_list<first_t<Vs...>>>;

}  // namespace soro::utls
