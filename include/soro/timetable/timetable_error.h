#pragma once

#include <system_error>

namespace soro::error {

enum class timetable {
  UNKNOWN_SOURCE_TYPE = 1,  // 0 is success
};

std::error_code make_error_code(timetable const e);

}  // namespace soro::error

namespace std {

template <>
struct is_error_code_enum<soro::error::timetable> : true_type {};

}  // namespace std
