#pragma once

#include <string_view>

#include "rapid/network.h"

namespace rapid {

network parse_network(std::string_view);

}  // namespace rapid