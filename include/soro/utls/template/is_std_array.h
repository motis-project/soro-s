#pragma once

#include <array>
#include <type_traits>

namespace soro::utls {

template <typename T>
struct is_std_array : std::false_type {};

template <typename V, std::size_t S>
struct is_std_array<std::array<V, S>> : std::true_type {};

template <typename T>
static constexpr auto is_std_array_v = is_std_array<T>::value;

}  // namespace soro::utls