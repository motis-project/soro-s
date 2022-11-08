#pragma once

#include <ostream>
#include "soro/floating_point_precision.h"

namespace soro {

#define UTIL_OP_TYPE_T_ h_km
#define MEMBER_NAME_ h_km_
#define UNIT_SYMBOL "[h/km]"

#include "soro/si/strong_types/operators/float.hxx"
#include "soro/si/strong_types/operators/operators.hxx"

#undef MEMBER_NAME_
#undef UTIL_OP_TYPE_T_
#undef UNIT_SYMBOL

}  // namespace soro
