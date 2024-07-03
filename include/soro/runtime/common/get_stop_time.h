#pragma once

#include "soro/timetable/train.h"

#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/signal_time.h"

namespace soro::runtime {

si::time get_stop_time_on_timetable_stop(interval const& interval,
                                         relative_time const arrival);

si::time get_stop_time_on_signal_stop(interval const& interval,
                                      relative_time const arrival,
                                      tt::train::trip const& trip,
                                      signal_time const& signal_time);

si::time get_stop_time(interval const& interval, relative_time const arrival,
                       tt::train::trip const& trip, signal_time const& st);

}  // namespace soro::runtime
