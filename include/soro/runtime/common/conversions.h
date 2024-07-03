#pragma once

#include "soro/base/time.h"

#include "soro/si/units.h"

namespace soro::runtime {

constexpr relative_time to_relative(si::time const t) {
  return relative_time(
      static_cast<relative_time::rep>(std::round(si::as_s(t))));
}

constexpr si::time to_si(relative_time const t) {
  return si::from_s(sc::duration_cast<seconds>(t).count());
}

}  // namespace soro::runtime