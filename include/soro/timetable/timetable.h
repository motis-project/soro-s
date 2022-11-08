#pragma once

#include <filesystem>

#include "soro/utls/serializable.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/base_timetable.h"
#include "soro/timetable/timetable_options.h"

namespace soro::tt {

enum class TimetableSource : uint8_t { NOT_FOUND, CSV, KSS };

struct timetable : utls::serializable<base_timetable> {
  using utls::serializable<base_timetable>::serializable;

  timetable(timetable_options const& opts, infra::infrastructure const& infra);

  auto begin() { return access_->begin(); }
  auto end() { return access_->end(); }

  auto begin() const { return access_->begin(); }
  auto end() const { return access_->end(); }

  auto size() const { return access_->size(); }

  auto const& operator[](std::size_t const idx) const {
    return (*access_)[idx];
  }

  TimetableSource source_type_{TimetableSource::NOT_FOUND};
};

inline auto begin(timetable const& tt) { return tt.begin(); }
inline auto end(timetable const& tt) { return tt.end(); }

}  // namespace soro::tt
