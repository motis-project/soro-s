#pragma once

#include "soro/infrastructure/graph/node.h"

namespace soro::runtime {

// TODO(julian) try to refactor the code so that the std::function becomes
// unnecessary
using TerminateCb = std::function<bool(infra::node::ptr)>;

}  // namespace soro::runtime
