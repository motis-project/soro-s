#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/base_timetable.h"
#include "soro/timetable/timetable_options.h"
#include "soro/utls/file/loaded_file.h"

namespace soro::tt {

bool is_csv_timetable(std::vector<utls::loaded_file> const& archive_files);
bool is_csv_timetable(timetable_options const& opts);

base_timetable parse_csv_timetable(timetable_options const& opts,
                                   infra::infrastructure const& infra);

}  // namespace soro::tt
