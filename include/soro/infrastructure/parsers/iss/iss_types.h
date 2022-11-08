#pragma once

#include <limits>

namespace soro::infra {

using rail_plan_node_id = uint64_t;

constexpr auto const INVALID_RP_NODE_ID =
    std::numeric_limits<rail_plan_node_id>::max();

}  // namespace soro::infra
