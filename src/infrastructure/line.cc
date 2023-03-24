#include "soro/infrastructure/line.h"

#include "utl/logging.h"

namespace soro::infra {

line::segment const& get_segment(line const& l, kilometrage const km) {
  utls::expect(!l.segments_.empty(), "Segments of line {} are empty.", l.id_);

  // ideally these would be a precondition, alas
  if (km < l.segments_.front().from_) {
    uLOG(utl::warn) << "Requesting line segment with kilometrage " << km
                    << ", but first line segment only starts at "
                    << l.segments_.front().from_ << ". Serving first segment.";
    return l.segments_.front();
  }

  if (km > l.segments_.back().to_) {
    uLOG(utl::warn) << "Requesting line segment with kilometrage " << km
                    << ", but last line segment only goes to "
                    << l.segments_.back().to_ << ". Serving last segment.";
    return l.segments_.back();
  }

  auto const it =
      std::lower_bound(
          std::begin(l.segments_), std::end(l.segments_), km,
          [](auto&& segment, auto&& v) { return segment.from_ <= v; }) -
      1;

  utls::sassert(it->from_ <= km && km <= it->to_,
                "Segment of line {} does not contain kilometrage {}.", l.id_,
                km);

  return *it;
}

bool line::has_signalling(kilometrage const km) const {
  return get_segment(*this, km).signalling_;
}

bool line::has_etcs(kilometrage const km) const {
  return get_segment(*this, km).etcs_;
}

}  // namespace soro::infra