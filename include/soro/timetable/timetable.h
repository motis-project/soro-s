#pragma once

#include <filesystem>

#include "soro/utls/serializable.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/base_timetable.h"
#include "soro/timetable/timetable_options.h"

namespace soro::tt {

enum class TimetableSource : uint8_t { NOT_FOUND, KSS };

struct timetable : utls::serializable<base_timetable> {
  using utls::serializable<base_timetable>::serializable;

  timetable(timetable_options const& opts, infra::infrastructure const& infra);

  TimetableSource source_type_{TimetableSource::NOT_FOUND};
};

}  // namespace soro::tt
