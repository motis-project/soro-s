#include "soro/timetable/parsers/kss/kss_error.h"

#include <string>
#include <system_error>

struct kss_error_category : std::error_category {
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

const char* kss_error_category::name() const noexcept { return "kss"; }

std::string kss_error_category::message(int ev) const {
  switch (static_cast<soro::error::kss>(ev)) {
    case soro::error::kss::STATION_NOT_FOUND:;
      return "could not find station from timetable in infrastructure";
    case soro::error::kss::STATION_ROUTE_NOT_FOUND:;
      return "could not find station route used in timetable in infrastructure";
    case soro::error::kss::NO_INTERLOCKING_ROUTE_PATH:
      return "could not determine interlocking route path";
    case soro::error::kss::STOP_IS_HALT_BUT_STATION_ROUTE_NO_HALT:
      return "stop is marked halt but station route does not have halt";
    case soro::error::kss::TRANSIT_TIME_BUT_NO_RUNTIME_CHECKPOINT:
      return "transit time is set but no runtime checkpoint in station route";
    case soro::error::kss::LZB_NOT_SUPPORTED: return "LZB is not supported";
    case soro::error::kss::FIRST_STOP_NO_HALT_NOT_SUPPORTED:
      return "first stop has no halt is not supported";
    case soro::error::kss::LAST_STOP_NO_HALT_NOT_SUPPORTED:
      return "last stop has no halt is not supported";
    case soro::error::kss::WORKED_ON_TRAIN: return "worked on train skipped";
    case soro::error::kss::SINGLE_STOP_NOT_SUPPORTED:
      return "single stop train is not supported";
    case soro::error::kss::BREAK_IN_NOT_SUPPORTED:
      return "break in not supported";
    case soro::error::kss::BREAK_OUT_NOT_SUPPORTED:
      return "break out not supported";
    case soro::error::kss::INFRASTRUCTURE_VERSION_MISMATCH:
      return "infrastructure version mismatch";
    case soro::error::kss::TRAIN_NUMBER_NOT_FOUND:
      return "train number not found";
    case soro::error::kss::TRAIN_NUMBER_AMBIGUOUS:
      return "train number ambiguous";
    case soro::error::kss::MANUALLY_REMOVED: return "train manually removed";
    case soro::error::kss::REMOVED_ENTIRE_STOP_SEQUENCE:
      return "entire stop sequence removed";
    case soro::error::kss::BREAK_IN_NOT_STARTING_ON_TRACK_END:
      return "break in not starting on track end";
    case soro::error::kss::BREAK_OUT_NOT_ENDING_ON_TRACK_END:
      return "break out not ending on track start";
    case soro::error::kss::ZLB_NOT_SUPPORTED:
      return "ZLB (=direct traffic control) not supported";
    case soro::error::kss::NO_ELEMENT_IN_ADDITIONAL_STOP:
      return "no element in additional stop";
    case soro::error::kss::STATION_NOT_FOUND_IN_ADDITIONAL_STOP:
      return "station not found in additional stop";
    case soro::error::kss::ADDITIONAL_STOPS_NOT_SUPPORTED:
      return "additional stops not supported";
    case soro::error::kss::ADDITIONAL_STOP_TYPE_NOT_SUPPORTED:
      return "additional stop type not supported";
    case soro::error::kss::ADDITIONAL_STOP_NAME_NOT_FOUND:
      return "additional stop name not found";
    case soro::error::kss::COULD_NOT_RESOLVE_SEQUENCE_POINT_NODE:
      return "could not resolve sequence point node";
    case soro::error::kss::OP_NOTES_NOT_SUPPORTED:
      return "op notes not supported";
  }

  return "not reachable";
}

const kss_error_category kss_error_category_singleton{};

const std::error_category& kss_error_category() {
  static struct kss_error_category const instance;
  return instance;
}

namespace soro::error {

std::error_code make_error_code(kss const e) {
  return {static_cast<int>(e), kss_error_category()};
}

}  // namespace soro::error
