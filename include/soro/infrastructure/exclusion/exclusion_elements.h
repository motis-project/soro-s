#pragma once

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

// contains the exclusion elements for each interlocking route
// [start, ..., end], i.e. a closed interval
soro::vector_map<interlocking_route::id, element::ids>
get_closed_exclusion_elements(infrastructure const& infra);

// contains the exclusion elements for each interlocking route, w/o start end
// (start, ..., end), i.e. an open interval
soro::vector_map<interlocking_route::id, element::ids>
get_open_exclusion_elements(
    soro::vector<element::ids> const& closed_exclusion_elements,
    infrastructure const& infra);

}  // namespace soro::infra
