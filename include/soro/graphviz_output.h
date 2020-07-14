#pragma once

#include <iosfwd>

#include "train.h"

namespace soro {

void graphiz_output(std::ostream&, timetable const&);

}  // namespace soro