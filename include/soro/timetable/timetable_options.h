#pragma once

#include <filesystem>

#include "cista/reflection/comparable.h"

namespace soro::tt {

struct timetable_options {
  CISTA_COMPARABLE()

  std::filesystem::path timetable_path_{""};
};

inline timetable_options make_timetable_opts(
    std::filesystem::path const& timetable_path) {
  return timetable_options{.timetable_path_ = timetable_path};
}

}  // namespace soro::tt
