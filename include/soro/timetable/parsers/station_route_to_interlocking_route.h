#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/rolling_stock/freight.h"
#include "soro/timetable/parsers/raw_to_trains.h"

namespace soro::tt {

soro::vector<infra::ir_ptr> get_interlocking_route_path(
    raw_train::run const& run, rs::FreightTrain const freight,
    infra::interlocking_subsystem const& ssr_man,
    infra::station_route_graph const& srg);

}  // namespace soro::tt
