#include "soro/timetable/parsers/kss/kss_error.h"

// namespace {

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
    case soro::error::kss::CTC_NOT_SUPPORTED: return "CTC is not supported";
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
  }

  return "not reachable";
}

const kss_error_category kss_error_category_singleton{};

//}  // namespace

const std::error_category& kss_error_category() {
  static struct kss_error_category const instance;
  return instance;
}

namespace soro::error {

std::error_code make_error_code(kss const e) {
  return {static_cast<int>(e), kss_error_category()};
}

}  // namespace soro::error
