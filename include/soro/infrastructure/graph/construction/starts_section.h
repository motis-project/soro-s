#pragma once

#include <type_traits>
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra {

inline mileage_dir section_pos_to_direction(section::position const pos) {
  utls::expect(is_boundary(pos), "only boundaries can be translated to dir");
  return is_start(pos) ? mileage_dir::rising : mileage_dir::falling;
}

}  // namespace soro::infra