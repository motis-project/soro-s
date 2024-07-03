#pragma once

#include <initializer_list>
#include <type_traits>

#include "soro/utls/template/first.h"

namespace soro::utls {

template <typename T>
struct is_initializer_list : std::false_type {};

template <typename T>
struct is_initializer_list<std::initializer_list<T>> : std::true_type {};

template <typename T>
constexpr static auto is_initializer_list_v = is_initializer_list<T>::value;

}  // namespace soro::utls
