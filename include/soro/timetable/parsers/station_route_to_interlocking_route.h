#pragma once

#include "soro/infrastructure/infrastructure.h"
#include "soro/rolling_stock/freight.h"
#include "soro/timetable/parsers/raw_to_trains.h"

namespace soro::tt {

soro::vector<infra::interlocking_route::id> get_interlocking_route_path(
    raw_train::run const& run, rs::FreightTrain freight,
    infra::infrastructure const& infra);

void print_ir_generating_failures();

}  // namespace soro::tt
