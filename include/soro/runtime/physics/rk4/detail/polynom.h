#pragma once

#include <array>

namespace soro::runtime::rk4::detail {

constexpr std::size_t poly_deg = 4;

template <typename T, std::size_t Degree>
using polynom = std::array<typename T::precision, Degree + 1>;

}  // namespace soro::runtime::rk4::detail