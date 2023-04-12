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
  switch (static_cast<route_transform_error>(ev)) {
    case route_transform_error::NO_FIRST_IR:
      return "could not find first interlocking route";
    case route_transform_error::NO_UNIQUE_FIRST_IR:
      return "could not find unique first interlocking route";
    case route_transform_error::NO_IR:
      return "could not find interlocking route";
    case route_transform_error::NO_UNIQUE_IR:
      return "could not find unique interlocking route";
    case route_transform_error::HALT_IN_SEQUENCE_BUT_NOT_IN_STATION_ROUTE:
      return "halt in sequence but not in station route";
  }
}

const route_transform_err_category route_transform_err_category_singleton{};

}  // namespace

std::error_code make_error_code(route_transform_error e) {
  return {static_cast<int>(e), route_transform_err_category_singleton};
}
