#include "soro/timetable/parsers/kss/kss_error.h"

namespace {

struct kss_error_category : std::error_category {
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

const char* kss_error_category::name() const noexcept { return "kss"; }

std::string kss_error_category::message(int ev) const {
  switch (static_cast<kss_error>(ev)) {
    case kss_error::NO_STATION_ROUTE_PATH:
      static_assert(false, "put text here");
    case kss_error::NO_INTERLOCKING_ROUTE_PATH:
      static_assert(false, "put text here");
    case kss_error::STOP_IS_HALT_BUT_STATION_ROUTE_NO_HALT:
      static_assert(false, "put text here");
    case kss_error::TRANSIT_TIME_BUT_NO_RUNTIME_CHECKPOINT:
      static_assert(false, "put text here");
    case kss_error::LZB_NOT_SUPPORTED: static_assert(false, "put text here");
    case kss_error::FIRST_STOP_NO_HALT_NOT_SUPPORTED:
      static_assert(false, "put text here");
    case kss_error::LAST_STOP_NO_HALT_NOT_SUPPORTED:
      static_assert(false, "put text here");
  }
}

const kss_error_category kss_error_category_singleton{};

}  // namespace

std::error_code make_error_code(kss_error e) {
  return {static_cast<int>(e), kss_error_category_singleton};
}
