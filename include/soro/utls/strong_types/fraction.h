#pragma once

#include <cmath>
#include <array>
#include <iostream>
#include <limits>
#include <optional>
#include <string>

#include "utl/verify.h"

#include "soro/base/fp_precision.h"

#include "soro/utls/strong_types/type_list.h"

namespace soro::utls {

/// --- base unit tags; correspond to the base SI units --- ///

// TODO(julian) These tags should not be in here to be honest
// fraction.h should only contain a generic fraction class

struct meter_tag {};
struct second_tag {};
struct kilogram_tag {};
struct ampere_tag {};

// --- angle tags --- ///

struct degree_tag {};
struct radian_tag {};

// --- minimum of two integer constants --- //

namespace detail {

template <typename Constant1, typename Constant2>
struct min : std::conditional_t<
                 (Constant1::value > Constant2::value),
                 std::integral_constant<std::size_t, Constant2::value>,
                 std::integral_constant<std::size_t, Constant1::value>> {};

}  // namespace detail

template <typename Constant1, typename Constant2>
inline auto constexpr min_v = detail::min<Constant1, Constant2>::value;

/// --- simplify a single tag --- ///

template <typename Nominator, typename Denominator, typename Tag>
struct simplify_tag {
  using nominator_count = count_type<Tag, Nominator>;
  using denominator_count = count_type<Tag, Denominator>;

  using simplified_nominator = std::conditional_t<
      nominator_count::value != 0 && denominator_count::value != 0,
      remove_times_t<min_v<nominator_count, denominator_count>, Tag, Nominator>,
      Nominator>;

  using simplified_denominator = std::conditional_t<
      nominator_count::value != 0 && denominator_count::value != 0,
      remove_times_t<min_v<nominator_count, denominator_count>, Tag,
                     Denominator>,
      Denominator>;
};

template <typename Nominator, typename Denominator, typename Tag>
using simplify_tag_n =
    typename simplify_tag<Nominator, Denominator, Tag>::simplified_nominator;

template <typename Nominator, typename Denominator, typename Tag>
using simplify_tag_d =
    typename simplify_tag<Nominator, Denominator, Tag>::simplified_denominator;

/// --- simplify all base tags (horrible implementation) --- ///
// TODO(julian) this implementation just calls simplify_tag on the
// meter/second/kilogram/ampere tag and should ideally be more generic and check
// for existing tags itself

template <typename N, typename D>
struct simplify {
  using nominator = simplify_tag_n<
      simplify_tag_n<
          simplify_tag_n<simplify_tag_n<N, D, meter_tag>, D, second_tag>, D,
          kilogram_tag>,
      D, ampere_tag>;

  using denominator = simplify_tag_d<
      N,
      simplify_tag_d<
          N, simplify_tag_d<N, simplify_tag_d<N, D, meter_tag>, second_tag>,
          kilogram_tag>,
      ampere_tag>;
};

template <typename TypeList1, typename TypeList2>
struct is_same_type_list {
  static constexpr bool const value =
      count_v<meter_tag, TypeList1> == count_v<meter_tag, TypeList2> &&
      count_v<second_tag, TypeList1> == count_v<second_tag, TypeList2> &&
      count_v<kilogram_tag, TypeList1> == count_v<kilogram_tag, TypeList2> &&
      count_v<ampere_tag, TypeList1> == count_v<ampere_tag, TypeList2>;
};

template <typename TypeList1, typename TypeList2>
static constexpr bool const is_same_type_list_v =
    is_same_type_list<TypeList1, TypeList2>::value;

template <typename Nominator, typename Denominator, typename Precision,
          typename Nominator2, typename Denominator2, typename Precision2>
struct is_same_fraction {
  static constexpr bool const value =
      std::is_same_v<Precision, Precision2> &&
      is_same_type_list_v<Nominator, Nominator2> &&
      is_same_type_list_v<Denominator, Denominator2>;
};

template <typename Nominator, typename Denominator, typename Precision,
          typename Nominator2, typename Denominator2, typename Precision2>
static constexpr inline bool const is_same_fraction_v =
    is_same_fraction<Nominator, Denominator, Precision, Nominator2,
                     Denominator2, Precision2>::value;

/// --- actual fraction implementation --- ///

template <typename Nominator, typename Denominator, typename Precision>
struct fraction {
  using precision = Precision;
  using value_type = precision;

  template <
      typename OtherN, typename OtherD, typename OtherP,
      std::enable_if_t<is_same_fraction_v<Nominator, Denominator, Precision,
                                          OtherN, OtherD, OtherP>,
                       bool> = true>
  operator fraction<OtherN, OtherD, OtherP>() {  // NOLINT
    return fraction<OtherN, OtherD, OtherP>{val_};
  }

  auto operator==(fraction const rhs) const {
    return soro::equal(val_, rhs.val_);
  }

  auto operator!=(fraction const rhs) const {
    return !soro::equal(val_, rhs.val_);
  }

  auto operator<(fraction const rhs) const { return val_ < rhs.val_; }
  auto operator>(fraction const rhs) const { return val_ > rhs.val_; }

  auto operator<=(fraction const rhs) const {
    return *this == rhs || val_ <= rhs.val_;
  }

  auto operator>=(fraction const rhs) const {
    return *this == rhs || val_ >= rhs.val_;
  }

  constexpr bool is_fraction() const {
    return !std::is_same_v<Denominator, type_list<>>;
  }

  constexpr bool is_unitless() const {
    return std::is_same_v<Nominator, type_list<>> &&
           std::is_same_v<Denominator, type_list<>>;
  }

  template <typename T>
  constexpr bool is() const {
    return std::is_same_v<fraction, T>;
  }

  static constexpr fraction zero() noexcept { return {precision{0}}; }

  static constexpr fraction infinity() noexcept {
    static_assert(std::numeric_limits<precision>::has_infinity);
    return {precision{std::numeric_limits<precision>::infinity()}};
  }

  static constexpr fraction min() noexcept {
    return {std::numeric_limits<precision>::lowest()};
  }

  static constexpr fraction max() noexcept {
    return {std::numeric_limits<precision>::max()};
  }

  static constexpr fraction invalid() noexcept {
    static_assert(std::numeric_limits<precision>::has_quiet_NaN);
    return {std::numeric_limits<precision>::quiet_NaN()};
  }

  constexpr bool is_zero() const noexcept { return *this == zero(); }
  constexpr bool is_positive() const noexcept { return val_ > 0.0; }
  constexpr bool is_negative() const noexcept { return val_ < 0.0; }
  constexpr bool is_infinity() const noexcept { return std::isinf(val_); }
  constexpr bool is_valid() const noexcept { return !std::isnan(val_); }
  constexpr bool is_nan() const noexcept { return std::isnan(val_); }

  std::string to_string() const {
    return std::to_string(val_) + get_fraction_unit_str(*this);
  }

  auto& operator+=(fraction const& rhs) {
    val_ += rhs.val_;
    return *this;
  }

  friend auto operator+(fraction lhs, fraction const& rhs) {
    lhs += rhs;
    return lhs;
  }

  auto& operator-=(fraction const& rhs) {
    val_ -= rhs.val_;
    return *this;
  }

  friend fraction operator-(fraction lhs, fraction const& rhs) {
    lhs -= rhs;
    return lhs;
  }

  auto operator-() const { return fraction{-val_}; }

  auto operator*=(Precision const unitless_scalar) {
    val_ *= unitless_scalar;
    return *this;
  }

  friend fraction operator*(Precision const lhs, fraction rhs) {
    rhs *= lhs;
    return rhs;
  }

  friend fraction operator*(fraction lhs, Precision const rhs) {
    lhs *= rhs;
    return lhs;
  }

  auto operator/=(Precision const unitless_scalar) {
    val_ /= unitless_scalar;
    return *this;
  }

  friend fraction operator/(Precision const lhs, fraction rhs) {
    rhs /= lhs;
    return rhs;
  }

  friend fraction operator/(fraction lhs, Precision const rhs) {
    lhs /= rhs;
    return lhs;
  }

  template <typename OtherN, typename OtherD, typename OtherP>
  constexpr auto operator*(fraction<OtherN, OtherD, OtherP> const& o) const {
    using simplified =
        simplify<concat_t<Nominator, OtherN>, concat_t<Denominator, OtherD>>;

    return fraction<typename simplified::nominator,
                    typename simplified::denominator, Precision>{val_ * o.val_};
  }

  template <typename OtherN, typename OtherD, typename OtherP>
  constexpr auto operator/(fraction<OtherN, OtherD, OtherP> const& o) const {
    using simplified =
        simplify<concat_t<Nominator, OtherD>, concat_t<Denominator, OtherN>>;

    return fraction<typename simplified::nominator,
                    typename simplified::denominator, Precision>{val_ / o.val_};
  }

  auto smooth() const { return fraction{soro::smooth(val_)}; }

  auto abs() const {
    return fraction<Nominator, Denominator, Precision>{std::abs(val_)};
  }

  template <std::size_t Exp>
  auto pow() const {
    using simplified = simplify<concat_times_t<Exp, Nominator>,
                                concat_times_t<Exp, Denominator>>;

    return fraction<typename simplified::nominator,
                    typename simplified::denominator, Precision>{
        std::pow(val_, static_cast<precision>(Exp))};
  }

  auto sqrt() const {
    constexpr auto nom_m_count = count_v<meter_tag, Nominator>;
    static_assert(nom_m_count % 2 == 0);
    using nom_m = concat_times_t<nom_m_count / 2, type_list<meter_tag>>;

    constexpr auto nom_s_count = count_v<second_tag, Nominator>;
    static_assert(nom_s_count % 2 == 0);
    using nom_s = concat_times_t<nom_s_count / 2, type_list<second_tag>>;

    constexpr auto nom_kg_count = count_v<kilogram_tag, Nominator>;
    static_assert((nom_kg_count % 2) == 0);
    using nom_kg = concat_times_t<nom_kg_count / 2, type_list<kilogram_tag>>;

    constexpr auto nom_a_count = count_v<ampere_tag, Nominator>;
    static_assert((nom_a_count % 2) == 0);
    using nom_a = concat_times_t<nom_a_count / 2, type_list<ampere_tag>>;

    using nominator = concat_t<nom_m, concat_t<nom_s, concat_t<nom_kg, nom_a>>>;

    constexpr auto de_m_count = count_v<meter_tag, Denominator>;
    static_assert(de_m_count % 2 == 0);
    using de_m = concat_times_t<de_m_count / 2, type_list<meter_tag>>;

    constexpr auto de_s_count = count_v<second_tag, Denominator>;
    static_assert(de_s_count % 2 == 0);
    using de_s = concat_times_t<de_s_count / 2, type_list<second_tag>>;

    constexpr auto de_kg_count = count_v<kilogram_tag, Denominator>;
    static_assert(de_kg_count % 2 == 0);
    using de_kg = concat_times_t<de_kg_count / 2, type_list<kilogram_tag>>;

    constexpr auto de_a_count = count_v<ampere_tag, Denominator>;
    static_assert(de_a_count % 2 == 0);
    using de_a = concat_times_t<de_a_count, type_list<ampere_tag>>;

    using denominator = concat_t<de_m, concat_t<de_s, concat_t<de_kg, de_a>>>;

    return fraction<nominator, denominator, Precision>{std::sqrt(val_)};
  }

  bool is_multiple_of(fraction const& other) const {
    auto const factor = *this / other;
    return (factor - round(factor)).abs() == decltype(factor)::zero();
  }

  Precision val_{std::numeric_limits<Precision>::quiet_NaN()};
};

template <typename Nominator, typename Denominator, typename Precision>
inline bool equal(fraction<Nominator, Denominator, Precision> const& f,
                  Precision const p) {
  return f == fraction<Nominator, Denominator, Precision>{p};
}

template <typename Nominator, typename Denominator, typename Precision>
inline bool equal(Precision const p,
                  fraction<Nominator, Denominator, Precision> const& f) {
  return f == fraction<Nominator, Denominator, Precision>{p};
}

template <typename Nominator, typename Denominator, typename Precision>
inline fraction<Nominator, Denominator, Precision> abs(
    fraction<Nominator, Denominator, Precision> const& f) {
  return f.abs();
}

template <typename Nominator, typename Denominator, typename Precision>
inline fraction<Nominator, Denominator, Precision> smooth(
    fraction<Nominator, Denominator, Precision> const& f) {
  return f.smooth();
}

template <std::size_t Exp, typename Fraction,
          std::enable_if_t<Exp == 1, bool> = true>
inline auto pow(Fraction const& f) {
  return f;
}

template <std::size_t Exp, typename Fraction,
          std::enable_if_t<Exp != 1, bool> = true>
inline auto pow(Fraction const& f) {
  return f * pow<Exp - 1>(f);
}

template <typename Fraction>
inline auto sqrt(Fraction const& f) {
  return f.sqrt();
}

template <typename Fraction>
inline Fraction round(Fraction const& f) {
  return Fraction{std::round(f.val_)};
}

// --- is_fraction --- //

template <typename T>
struct is_fraction : std::false_type {};

template <typename N, typename D, typename P>
struct is_fraction<fraction<N, D, P>> : std::true_type {};

template <typename T>
constexpr bool const is_fraction_v = is_fraction<T>::value;

template <typename T>
concept Fraction = is_fraction_v<T>;

/// --- support for operator<< --- ///
// TODO(julian) make this constexpr when all std libs made std::string constexpr

template <typename Tag>
inline std::string get_tag_symbol() {
  if constexpr (std::is_same_v<Tag, meter_tag>) {
    return "m";
  } else if constexpr (std::is_same_v<Tag, second_tag>) {
    return "s";
  } else if constexpr (std::is_same_v<Tag, kilogram_tag>) {
    return "kg";
  } else if constexpr (std::is_same_v<Tag, ampere_tag>) {
    return "A";
  }
}

std::vector<std::string> const exponent_symbols = {"",  "",  "²", "³", "⁴",
                                                   "⁵", "⁶", "⁷", "⁸", "⁹"};

template <typename TypeList>
constexpr std::optional<std::string> get_type_list_unit_str() {
  std::string result;

  auto const get_str_for_tag = []<typename Tag>(Tag const&) {
    if constexpr (auto const count = count_v<Tag, TypeList>;
                  count != 0 && count < 10) {
      return fmt::format("{}{}*", get_tag_symbol<Tag>(),
                         exponent_symbols[count]);
    } else {
      return "";
    }
  };

  result += get_str_for_tag(meter_tag{});
  result += get_str_for_tag(second_tag{});
  result += get_str_for_tag(kilogram_tag{});
  result += get_str_for_tag(ampere_tag{});

  if (result.empty()) {
    return {};
  }

  if (result.back() == '*') {
    result.pop_back();
  }

  return std::optional<std::string>{result};
}

template <typename Nominator, typename Denominator, typename Precision>
constexpr std::string get_fraction_unit_str(
    fraction<Nominator, Denominator, Precision> const& fraction) {
  auto const nominator_str = get_type_list_unit_str<Nominator>();

  if (!fraction.is_fraction()) {
    return fmt::format("[{}]", nominator_str.value_or(""));
  } else {
    return fmt::format("[{}/{}]", nominator_str.value_or("1"),
                       get_type_list_unit_str<Denominator>().value_or(""));
  }
}

template <typename Nominator, typename Denominator, typename Precision>
std::ostream& operator<<(std::ostream& os,
                         fraction<Nominator, Denominator, Precision> f) {
  os << f.to_string();
  return os;
}

}  // namespace soro::utls
