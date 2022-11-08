#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"
#include "soro/utls/unixtime.h"

namespace soro::simulation {

namespace tt = soro::tt;

struct route_usage {
  utls::unixtime from_{utls::INVALID_TIME};
  utls::unixtime to_{utls::INVALID_TIME};
  tt::train::id train_id_{tt::train::INVALID};
};

using usage_index = size_t;

// for every signal station route a list of trains, ordered by usage
using route_ordering = std::vector<std::vector<route_usage>>;

route_ordering get_route_ordering(infra::infrastructure const& infra,
                                  tt::timetable const& tt);

usage_index get_usage_index(route_ordering const& ordering, tt::train::id train,
                            infra::interlocking_route::id ssr);

}  // namespace soro::simulation
