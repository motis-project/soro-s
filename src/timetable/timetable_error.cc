#include "soro/timetable/timetable_error.h"

namespace {

struct timetable_error_category : std::error_category {
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

const char* timetable_error_category::name() const noexcept { return "tt"; }

std::string timetable_error_category::message(int ev) const {
  switch (static_cast<soro::error::timetable>(ev)) {
    case soro::error::timetable::UNKNOWN_SOURCE_TYPE:;
      return "could not determine source type for timetable";
  }

  return "not reachable";
}

const timetable_error_category kss_error_category_singleton{};

}  // namespace

namespace soro::error {

std::error_code make_error_code(timetable const e) {
  return {static_cast<int>(e), kss_error_category_singleton};
}

}  // namespace soro::error
