#pragma once

#include "soro/utls/file/loaded_file.h"
#include "soro/utls/result.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/base_timetable.h"
#include "soro/timetable/timetable_options.h"

namespace soro::tt {

bool is_kss_timetable(timetable_options const& opts);

utls::result<base_timetable> parse_kss_timetable(
    timetable_options const& opts, infra::infrastructure const& infra);

}  // namespace soro::tt
