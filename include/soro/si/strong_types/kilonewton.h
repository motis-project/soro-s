#pragma once

#include "soro/floating_point_precision.h"

namespace soro {

#define UTIL_OP_TYPE_T_ kn
#define MEMBER_NAME_ kn_
#define UNIT_SYMBOL "[kn]"

#include "soro/si/strong_types/operators/float.hxx"
#include "soro/si/strong_types/operators/operators.hxx"

#undef MEMBER_NAME_
#undef UTIL_OP_TYPE_T_
#undef UNIT_SYMBOL

}  // namespace soro
