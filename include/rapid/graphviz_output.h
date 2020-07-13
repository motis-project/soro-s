#pragma once

#include <iosfwd>

#include "train.h"

namespace rapid {

void graphiz_output(std::ostream&, timetable const&);

}  // namespace rapid