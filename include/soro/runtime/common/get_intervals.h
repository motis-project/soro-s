#pragma once

#include "soro/timetable/train.h"

#include "soro/runtime/common/interval.h"

namespace soro::runtime {

intervals get_intervals(tt::train const& train,
                        infra::type_set const& record_types,
                        infra::infrastructure const& infra);

}  // namespace soro::runtime
