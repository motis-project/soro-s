#pragma once

#include <string_view>

#include "soro/network.h"

namespace soro {

network parse_network(std::string_view);

}  // namespace soro