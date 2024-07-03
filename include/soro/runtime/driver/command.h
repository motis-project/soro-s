#pragma once

#include "soro/runtime/common/runtime_result.h"

namespace soro::runtime {

struct command : train_state {
  enum class action : uint8_t {
    accelerate,
    brake,
    coast,
    cruise,
    halt,
    invalid
  };

  command() = default;
  command(action const a, train_state const& resulting_state)
      : train_state{resulting_state}, action_{a} {}

  action action_{action::invalid};
};

}  // namespace soro::runtime