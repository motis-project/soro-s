#pragma once

#include <system_error>

enum class kss_error {
  STATION_NOT_FOUND = 1,  // 0 is success
  WORKED_ON_TRAIN,
  NO_STATION_ROUTE_PATH,
  NO_INTERLOCKING_ROUTE_PATH,
  STOP_IS_HALT_BUT_STATION_ROUTE_NO_HALT,
  TRANSIT_TIME_BUT_NO_RUNTIME_CHECKPOINT,
  CTC_NOT_SUPPORTED,
  SINGLE_STOP_NOT_SUPPORTED,
  BREAK_IN_NOT_SUPPORTED,
  BREAK_OUT_NOT_SUPPORTED,
  FIRST_STOP_NO_HALT_NOT_SUPPORTED,
  LAST_STOP_NO_HALT_NOT_SUPPORTED,
};

std::error_code make_error_code(kss_error e);

namespace std {

template <>
struct is_error_code_enum<kss_error> : true_type {};

}  // namespace std
