#pragma once

namespace soro::simulation {

enum class enable_halt_dists : bool { Off, On };
enum class enable_runtime_dists : bool { Off, On };

struct simulation_options {
  bool use_halt_dists() const {
    return enable_halt_dists_ == enable_halt_dists::On;
  }

  bool use_runtime_dists() const {
    return enable_runtime_dists_ == enable_runtime_dists::On;
  }

  enable_halt_dists enable_halt_dists_{enable_halt_dists::On};
  enable_runtime_dists enable_runtime_dists_{enable_runtime_dists::On};
};

}  // namespace soro::simulation
