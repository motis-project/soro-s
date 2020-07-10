#pragma once

#include "rapid/train.h"

namespace rapid {

struct network;

timetable parse_timetable(network const&, std::string_view trains,
                          std::string_view timetable);

}  // namespace rapid