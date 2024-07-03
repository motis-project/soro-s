#pragma once

#include <type_traits>

namespace soro::utls {

template <typename T>
struct always_false : std::false_type {};

template <typename T>
constexpr auto always_false_v = always_false<T>::value;

}  // namespace soro::utls