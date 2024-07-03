#pragma once

#include "soro/utls/template/type_at_position.h"

namespace soro::utls {

template <typename... Ts>
using first_t = typename type_at_position<0, Ts...>::type;

}  // namespace soro::utls
