#pragma once

#include "soro/base/soro_types.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra {

struct line {
  using id = uint16_t;

  struct segment {
    kilometrage from_{si::INVALID<kilometrage>};
    kilometrage to_{si::INVALID<kilometrage>};
    bool etcs_{false};
    bool lzb_{false};
    bool signalling_{false};
  };

  segment const& get_segment(kilometrage const km) const {
    utls::sassert(
        km >= segments_.front().from_,
        "Kilometrage {} less than first kilometrage value {} of line {}.", km,
        id_, segments_.front().from_);
    utls::sassert(
        km <= segments_.back().to_,
        "Kilometrage {} more than last kilometrage value {} of line {}.", km,
        id_, segments_.back().to_);
    utls::sassert(!segments_.empty(), "Segments of line {} are empty.", id_);

    auto const it =
        std::lower_bound(
            std::begin(segments_), std::end(segments_), km,
            [](auto&& segment, auto&& v) { return segment.from_ < v; }) -
        1;

    utls::sassert(it->from_ <= km && km <= it->to_,
                  "Segment of line {} does not contain kilometrage {}.", id_,
                  km);

    return *it;
  }

  bool has_signalling(kilometrage const km) const {
    return get_segment(km).signalling_;
  }

  bool has_etcs(kilometrage const km) const { return get_segment(km).etcs_; }

  id id_;

  // ordered w.r.t. distance
  soro::vector<segment> segments_;
};

}  // namespace soro::infra
