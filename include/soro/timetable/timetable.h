#pragma once

#include <filesystem>

#include "soro/utls/result.h"
#include "soro/utls/serializable.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/base_timetable.h"
#include "soro/timetable/timetable_options.h"

namespace soro::tt {

enum class timetable_source : uint8_t { NOT_FOUND, KSS };

struct timetable : utls::serializable<base_timetable> {
  using utls::serializable<base_timetable>::serializable;

  using optional_ptr = soro::optional<timetable const*>;

  explicit timetable(base_timetable&& bt);
  timetable(timetable_options const& opts, infra::infrastructure const& infra);

  timetable_source source_type_{timetable_source::NOT_FOUND};
};

utls::result<timetable> try_parsing_timetable(
    timetable_options const& opts, infra::infrastructure const& infra);

}  // namespace soro::tt
