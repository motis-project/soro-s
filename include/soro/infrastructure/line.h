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

  bool has_signalling(kilometrage const km) const;
  bool has_etcs(kilometrage const km) const;

  id id_;

  // ordered w.r.t. distance
  soro::vector<segment> segments_;
};

using lines = soro::map<line::id, line>;

}  // namespace soro::infra
