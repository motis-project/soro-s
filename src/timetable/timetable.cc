#include "soro/timetable/timetable.h"

#include "soro/timetable/parsers/kss/parse_kss_timetable.h"

namespace soro::tt {

TimetableSource get_source_type(timetable_options const& opts) {
  if (is_kss_timetable(opts)) {
    return TimetableSource::KSS;
  }

  return TimetableSource::NOT_FOUND;
}

timetable::timetable(timetable_options const& opts,
                     infra::infrastructure const& infra)
    : source_type_{get_source_type(opts)} {

  switch (source_type_) {
    case TimetableSource::KSS: {
      mem_ = parse_kss_timetable(opts, infra);
      break;
    }
    case TimetableSource::NOT_FOUND: {
      throw utl::fail("Could not determine timetable source type for path {}",
                      opts.timetable_path_);
    }
  }

  access_ = std::addressof(std::get<base_timetable>(mem_));
}

}  // namespace soro::tt
