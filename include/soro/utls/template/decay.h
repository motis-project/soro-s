#pragma once

#include <type_traits>

namespace soro::utls {

template <typename T>
using decay_t = std::remove_reference_t<std::remove_const_t<T>>;

}  // namespace soro::utls
