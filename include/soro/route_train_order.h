#pragma once

#include "cista/containers/hash_map.h"
#include "cista/containers/pair.h"

#include "soro/train.h"

namespace soro {

using train_order_map = cista::raw::hash_map<route::id_t, std::vector<route*>>;

void compute_route_train_order(timetable const&);

}  // namespace soro