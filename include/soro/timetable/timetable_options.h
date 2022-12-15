#pragma once

#include <filesystem>

#include "cista/reflection/comparable.h"

#include "soro/utls/unixtime.h"

namespace soro::tt {

// Parse a timetable from the following:
//   - 'Timetable.xml', a single dispo timetable xml
//   - 'timetable.fpl', which is a .tar.zst file containing timetable xmls
//   - 'timetable/', a folder which contains a multiple timetable xmls

// All train runs with an event in the interval given by start_ and end_
// will be indexed and put into train_runs_ in the resulting timetable

struct timetable_options {
  CISTA_COMPARABLE()

  std::filesystem::path timetable_path_{""};
  utls::unixtime start_{utls::EPOCH};
  utls::unixtime end_{utls::END_OF_TIME};
};

}  // namespace soro::tt
