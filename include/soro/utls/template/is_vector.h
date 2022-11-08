#pragma once

#include <type_traits>

namespace soro::utls {

template <typename T>
struct is_vector : std::false_type {};

template <typename V>
struct is_vector<soro::vector<V>> : std::true_type {};

template <typename T>
inline constexpr auto const is_vector_v = is_vector<T>::value;

}  // namespace soro::utls
