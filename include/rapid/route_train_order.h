#pragma once

#include "cista/containers/hash_map.h"
#include "cista/containers/pair.h"

#include "rapid/train.h"

namespace rapid {

using route_id_t = cista::raw::pair<node*, node*>;

using train_order_map = cista::raw::hash_map<route_id_t, std::vector<route*>>;

train_order_map compute_route_train_order(timetable const&);

}  // namespace rapid