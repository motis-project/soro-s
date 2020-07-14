#pragma once

#include "soro/train.h"

namespace soro {

struct network;

timetable parse_timetable(network const&, std::string_view trains,
                          std::string_view timetable);

}  // namespace soro