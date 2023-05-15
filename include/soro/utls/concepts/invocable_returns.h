#pragma once

#include <concepts>
#include <type_traits>

namespace soro::utls {

template <typename Invocable, typename ReturnType, typename... ParameterTypes>
concept invocable_returns =
    std::invocable<Invocable, ParameterTypes...> &&
    std::is_same_v<std::invoke_result_t<Invocable, ParameterTypes...>,
                   ReturnType>;

}  // namespace soro::utls
