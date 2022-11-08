#pragma once

#include <tuple>
#include <type_traits>

#include "cista/containers/tuple.h"

namespace soro::utls::tuple {

// --- is_tuple --- //

template <typename... Pack>
struct is_tuple : std::false_type {};

template <typename... Pack>
struct is_tuple<std::tuple<Pack...>> : std::true_type {};

template <typename... Pack>
struct is_tuple<cista::tuple<Pack...>> : std::true_type {};

template <typename Tuple>
constexpr bool is_tuple_v = is_tuple<std::remove_const_t<Tuple>>::value;

}  // namespace soro::utls::tuple
