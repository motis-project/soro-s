#pragma once

#include <cstddef>
#include <type_traits>

namespace soro::utls {

template <typename...>
struct type_list;

/// --- count --- ///

template <typename Needle, typename... Haystack>
struct count_type {};

template <typename Needle, typename... Haystack>
struct count_type<Needle, type_list<Haystack...>>
    : std::integral_constant<size_t,
                             (0 + ... + std::is_same_v<Needle, Haystack>)> {};

template <typename Needle, typename... Haystack>
inline constexpr auto count_v = count_type<Needle, Haystack...>::value;

/// --- push type to front of list --- ///

template <typename...>
struct push_front;

template <typename ToAdd, typename... Types>
struct push_front<ToAdd, type_list<Types...>> {
  using type =
      std::conditional_t<std::is_same_v<ToAdd, void>, type_list<Types...>,
                         type_list<ToAdd, Types...>>;
};

template <typename ToAdd, typename... Types>
using push_front_t = typename push_front<ToAdd, Types...>::type;

/// --- remove type from list at most once --- ///

template <typename...>
struct remove_once;

template <typename Needle>
struct remove_once<Needle, type_list<>> {
  using type = type_list<>;
};

template <typename Needle, typename LastHayblade>
struct remove_once<Needle, type_list<LastHayblade>> {
  using type = std::conditional_t<std::is_same_v<Needle, LastHayblade>,
                                  type_list<>, type_list<LastHayblade>>;
};

template <typename Needle, typename Hayblade, typename... Haystack>
struct remove_once<Needle, type_list<Hayblade, Haystack...>> {
  using type = std::conditional_t<
      std::is_same_v<Needle, Hayblade>, type_list<Haystack...>,
      push_front_t<Hayblade,
                   typename remove_once<Needle, type_list<Haystack...>>::type>>;
};

template <typename Needle, typename... Haystack>
using remove_once_t = typename remove_once<Needle, Haystack...>::type;

/// --- actual type list --- ///

template <typename... Ts>
struct type_list {
  template <typename... OtherTs>
  type_list<Ts..., OtherTs...> concat(type_list<OtherTs...>) const {}
};

/// --- remove type at most RemoveCounter times from list --- ///

template <size_t RemoveCounter, typename Needle, typename... Haystack>
struct remove_times {};

template <typename Needle, typename... Haystack>
struct remove_times<0, Needle, type_list<Haystack...>> {
  using type = type_list<Haystack...>;
};

template <size_t RemoveCounter, typename Needle, typename... Haystack>
struct remove_times<RemoveCounter, Needle, type_list<Haystack...>> {
  using type = typename remove_times<
      RemoveCounter - 1, Needle,
      remove_once_t<Needle, type_list<Haystack...>>>::type;
};

template <size_t RemoveCounter, typename Needle, typename... Haystack>
using remove_times_t =
    typename remove_times<RemoveCounter, Needle, Haystack...>::type;

/// --- concat two type lists --- ///

template <typename TypeList1, typename TypeList2>
using concat_t =
    decltype(std::declval<TypeList1>().concat(std::declval<TypeList2>()));

// --- concat_times --- //

template <size_t Times, typename TypeList1>
struct concat_times;

template <typename TypeList1>
struct concat_times<0, TypeList1> {
  using type = type_list<>;
};

template <size_t Times, typename TypeList1>
struct concat_times {
  using type =
      concat_t<TypeList1, typename concat_times<Times - 1, TypeList1>::type>;
};

template <size_t Times, typename TypeList1>
using concat_times_t = typename concat_times<Times, TypeList1>::type;

}  // namespace soro::utls
