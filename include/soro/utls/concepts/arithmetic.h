#pragma once

#include <concepts>

namespace soro::utls {

template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

}  // namespace soro::utls
