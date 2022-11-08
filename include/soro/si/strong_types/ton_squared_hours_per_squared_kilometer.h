#pragma once

#include "soro/floating_point_precision.h"

namespace soro {

#define UTIL_OP_TYPE_T_ th2_km2
#define MEMBER_NAME_ th2_km2_
#define UNIT_SYMBOL "[t*h^2/km^2]"

#include "soro/si/strong_types/operators/float.hxx"
#include "soro/si/strong_types/operators/operators.hxx"

#undef MEMBER_NAME_
#undef UTIL_OP_TYPE_T_
#undef UNIT_SYMBOL

}  // namespace soro
