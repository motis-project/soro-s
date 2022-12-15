#pragma once

#include <concepts>

namespace soro::utls {

template <typename T, typename... Ts>
concept is_any_of = (std::same_as<T, Ts> || ...);

}  // namespace soro::utls