#pragma once

#include <vector>

#include "soro/utls/coroutine/generator.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/element.h"

namespace soro::infra {

struct section {
  using id = uint32_t;

  utls::generator<const element_ptr> iterate(rising const direction) const;

  soro::vector<element_ptr> elements_;
  si::length length_{si::ZERO<si::length>};
  line_id line_id_{INVALID_LINE_ID};
};

}  // namespace soro::infra
