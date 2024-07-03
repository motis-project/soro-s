#pragma once

#include <limits>

#include "soro/base/soro_types.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra {

struct line {
  using id = uint16_t;

  constexpr static id invalid() { return std::numeric_limits<id>::max(); }

  struct segment {
    kilometrage from_{kilometrage::invalid()};
    kilometrage to_{kilometrage::invalid()};
    bool etcs_{false};
    bool lzb_{false};
    bool signalling_{false};
  };

  bool has_signalling(kilometrage const km) const;
  bool has_etcs(kilometrage const km) const;
  bool has_lzb(kilometrage const km) const;

  id id_;

  // ordered w.r.t. distance
  soro::vector<segment> segments_;
};

using lines = soro::map<line::id, line>;

}  // namespace soro::infra
