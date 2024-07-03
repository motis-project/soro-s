#pragma once

#include "soro/base/soro_types.h"
#include "soro/si/units.h"

namespace soro::infra {

// fwd decl
struct infrastructure;

using brake_path_key = soro::array<char, 2>;

using brake_path = soro::strong<uint8_t, struct _brake_path_id>;

}  // namespace soro::infra
