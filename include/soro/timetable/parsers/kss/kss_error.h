#pragma once

#include <system_error>

namespace soro::error {

enum class kss {
  STATION_NOT_FOUND = 1,  // 0 is success
  STATION_ROUTE_NOT_FOUND,
  WORKED_ON_TRAIN,
  NO_INTERLOCKING_ROUTE_PATH,
  STOP_IS_HALT_BUT_STATION_ROUTE_NO_HALT,
  TRANSIT_TIME_BUT_NO_RUNTIME_CHECKPOINT,
  CTC_NOT_SUPPORTED,
  SINGLE_STOP_NOT_SUPPORTED,
  BREAK_IN_NOT_SUPPORTED,
  BREAK_OUT_NOT_SUPPORTED,
  FIRST_STOP_NO_HALT_NOT_SUPPORTED,
  LAST_STOP_NO_HALT_NOT_SUPPORTED,
  INFRASTRUCTURE_VERSION_MISMATCH,
  TRAIN_NUMBER_NOT_FOUND,
  TRAIN_NUMBER_AMBIGUOUS
};

std::error_code make_error_code(kss const e);

}  // namespace soro::error

namespace std {

template <>
struct is_error_code_enum<soro::error::kss> : true_type {};

}  // namespace std
