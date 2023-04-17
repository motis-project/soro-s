#include "soro/timetable/timetable.h"

#include "soro/utls/result.h"
#include "soro/utls/sassert.h"

#include "soro/timetable/base_timetable.h"
#include "soro/timetable/parsers/kss/parse_kss_timetable.h"
#include "soro/timetable/timetable_error.h"

namespace soro::tt {

timetable_source get_source_type(timetable_options const& opts) {
  if (is_kss_timetable(opts)) {
    return timetable_source::KSS;
  }

  return timetable_source::NOT_FOUND;
}

timetable::timetable(base_timetable&& bt) {
  mem_ = std::move(bt);
  access_ = std::addressof(std::get<base_timetable>(mem_));
}

timetable::timetable(timetable_options const& opts,
                     infra::infrastructure const& infra)
    : source_type_{get_source_type(opts)} {

  switch (source_type_) {
    case timetable_source::KSS: {
      auto kss = parse_kss_timetable(opts, infra);
      if (!kss) {
        throw utl::fail("kss parser failed: {}", kss.error().message());
      }

      mem_ = std::move(*kss);
      break;
    }
    case timetable_source::NOT_FOUND: {
      throw utl::fail("could not determine timetable source type for path {}",
                      opts.timetable_path_);
    }
  }

  access_ = std::addressof(std::get<base_timetable>(mem_));
}

utls::result<timetable> try_parsing_timetable(
    timetable_options const& opts, infra::infrastructure const& infra) {
  utls::result<base_timetable> bt;

  auto const source_type = get_source_type(opts);

  switch (source_type) {
    case timetable_source::KSS: {
      bt = parse_kss_timetable(opts, infra);

      if (!bt) {
        return utls::propagate(bt);
      }

      break;
    }

    case timetable_source::NOT_FOUND: {
      return std::unexpected(error::timetable::UNKNOWN_SOURCE_TYPE);
    }
  }

  utls::sassert(bt.has_value(), "base timetable not successfully parsed");

  return timetable(std::move(*bt));
}

}  // namespace soro::tt
