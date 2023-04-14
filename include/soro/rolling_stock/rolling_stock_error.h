#pragma once

#include <system_error>

namespace soro::error {

enum class rolling_stock {
  TRACTION_VEHICLE_NOT_FOUND = 1,  // 0 is success
};

std::error_code make_error_code(rolling_stock const e);

}  // namespace soro::error

namespace std {

template <>
struct is_error_code_enum<soro::error::rolling_stock> : true_type {};

}  // namespace std
