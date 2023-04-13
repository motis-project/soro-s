#include "soro/timetable/parsers/route_transform_error.h"

namespace {

struct route_transform_err_category : std::error_category {
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

const char* route_transform_err_category::name() const noexcept {
  return "route_transform";
}

std::string route_transform_err_category::message(int ev) const {
  switch (static_cast<soro::error::route_transform>(ev)) {
    case soro::error::route_transform::NO_FIRST_IR:
      return "could not find first interlocking route";
    case soro::error::route_transform::NO_UNIQUE_FIRST_IR:
      return "could not find unique first interlocking route";
    case soro::error::route_transform::NO_IR:
      return "could not find interlocking route";
    case soro::error::route_transform::NO_UNIQUE_IR:
      return "could not find unique interlocking route";
    case soro::error::route_transform::
        HALT_IN_SEQUENCE_BUT_NOT_IN_STATION_ROUTE:
      return "halt in sequence but not in station route";
  }

  return "not reachable";
}

const route_transform_err_category route_transform_err_category_singleton{};

}  // namespace

namespace soro::error {

std::error_code make_error_code(route_transform const e) {
  return {static_cast<int>(e), route_transform_err_category_singleton};
}

}  // namespace soro::error
