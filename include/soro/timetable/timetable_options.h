#pragma once

#include <filesystem>

#include "cista/reflection/comparable.h"

#include "soro/utls/unixtime.h"

namespace soro::tt {

// Parse a timetable from the following:
//   - 'Timetable.xml', a single dispo timetable xml
//   - 'timetable.fpl', which is a .tar.zst file containing timetable xmls
//   - 'timetable/', a folder which contains a multiple timetable xmls

struct timetable_options {
  CISTA_COMPARABLE()

  std::filesystem::path timetable_path_{""};
};

}  // namespace soro::tt
