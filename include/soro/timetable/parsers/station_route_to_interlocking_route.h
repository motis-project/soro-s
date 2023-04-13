#pragma once

#include "soro/utls/result.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/rolling_stock/freight.h"
#include "soro/timetable/stop_sequence.h"

namespace soro::tt {

struct interlocking_transformation {
  soro::vector<infra::interlocking_route::id> path_;
  soro::vector<sequence_point> sequence_points_;
};

utls::result<interlocking_transformation> transform_to_interlocking(
    stop_sequence const& stop_sequence, rs::FreightTrain freight,
    infra::infrastructure const& infra);

void print_ir_generating_failures();

}  // namespace soro::tt
