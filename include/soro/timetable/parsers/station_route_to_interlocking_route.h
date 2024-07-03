#pragma once

#include "soro/utls/result.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/train.h"

namespace soro::tt {

utls::result<soro::vector<infra::interlocking_route::id>>
transform_to_interlocking(train const& train,
                          infra::infrastructure const& infra);

}  // namespace soro::tt
