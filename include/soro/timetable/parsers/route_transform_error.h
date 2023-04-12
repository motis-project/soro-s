#pragma once

#include <system_error>

enum class route_transform_error {
  NO_FIRST_IR = 1,  // start at 1 since 0 is success
  NO_UNIQUE_FIRST_IR,
  NO_IR,
  NO_UNIQUE_IR,
  HALT_IN_SEQUENCE_BUT_NOT_IN_STATION_ROUTE
};

std::error_code make_error_code(route_transform_error e);

namespace std {

template <>
struct is_error_code_enum<route_transform_error> : true_type {};

}  // namespace std
