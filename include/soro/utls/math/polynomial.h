#pragma once

#include <cinttypes>
#include <cmath>
#include <any>
#include <array>
#include <type_traits>
#include <utility>

#include "soro/base/soro_types.h"
#include "soro/utls/template/decay.h"
#include "soro/utls/template/type_at_position.h"
#include "soro/utls/tuple/apply_at.h"

namespace soro::utls {

// --- get_precision<T> --- //
// returns T if T is floating point or T::precision

template <typename T>
struct get_precision {
  using type = typename decay_t<T>::precision;
};

template <>
struct get_precision<double> {
  using type = double;
};

template <>
struct get_precision<float> {
  using type = float;
};

template <typename T>
using get_precision_t = typename get_precision<T>::type;

// --- polynomial function implementation --- //

template <typename...>
struct polynomial;

template <>
struct polynomial<> {
  constexpr polynomial() = default;

  template <typename X>
  constexpr auto operator()(X const) const {
    return X{0};
  }
};

template <typename... Factors>
struct polynomial {
  template <typename InputT>
  using result_t =
      decltype(std::declval<polynomial<Factors...>>()(std::declval<InputT>()));
  using precision = get_precision_t<first_t<Factors...>>;

  // factors in descending order of degree,
  // e.g. polynomial(a, b, c, d) -> ax³ + bx² + cx + d
  // a polynomial of degree 3 requires all 4 factors, even when 0.
  constexpr auto degree() const { return sizeof...(Factors) - 1; }

  /*
   * The implementation of operator() is a bit of a hack for now.
   * Since we have a strong typing system in place the return type
   * of a pow(x, exp) is depending on the value of exp. This makes
   * evaluating a polynomial a major headache.
   *
   * The alternative is evaluating a polynomial via Horner's method, making a
   * pow function unnecessary, but the type of the accumulator variable
   * changes during execution of the algorithm, making it also quite tiresome.
   *
   * The current hack simply detects a strong typed value and computes
   * everything in the 'unsafe' type mode via accessing .val_ and casting to the
   * correct return type at the end.
   *
   */

  template <typename X,
            std::enable_if_t<!std::is_floating_point_v<X>, bool> = true>
  auto operator()(X const x) const {
    typename X::precision result = get<0>(factors_).val_;

    for (size_t i = 1; i < sizeof...(Factors); ++i) {
      tuple::apply_at(factors_, i, [&](auto const& f) {
        result = f.val_ + (result * x.val_);
      });
    }

    return std::tuple_element_t<sizeof...(Factors) - 1, std::tuple<Factors...>>{
        result};
  }

  template <typename X,
            std::enable_if_t<std::is_floating_point_v<X>, bool> = true>
  auto operator()(X const x) const {
    X result = get<0>(factors_);

    for (size_t i = 1; i < sizeof...(Factors); ++i) {
      tuple::apply_at(factors_, i,
                      [&](auto const& f) { result = f + (result * x); });
    }

    return result;
  }

  template <typename FactorType>
  friend auto operator*(polynomial lhs, FactorType const& rhs) {
    lhs *= rhs;
    return lhs;
  }

  soro::tuple<Factors...> factors_;
};

template <typename... Factors>
polynomial(Factors...) -> polynomial<Factors...>;

template <typename... Factors>
auto make_polynomial(Factors&&... factors) {
  return polynomial<decay_t<Factors>...>{
      .factors_ = soro::tuple<decay_t<Factors>...>{std::move(factors)...}};
}

}  // namespace soro::utls