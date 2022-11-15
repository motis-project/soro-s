#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/base_timetable.h"
#include "soro/timetable/timetable_options.h"
#include "soro/utls/file/loaded_file.h"

namespace soro::tt {

bool is_kss_timetable(timetable_options const& opts);

base_timetable parse_kss_timetable(timetable_options const& opts,
                                   infra::infrastructure const& infra);

}  // namespace soro::tt
