#pragma once

namespace soro::simulation {

enum class EnableHaltDists : bool { Off, On };
enum class EnableRuntimeDists : bool { Off, On };

struct simulation_options {

  bool use_halt_dists() const {
    return enable_halt_dists_ == EnableHaltDists::On;
  }

  bool use_runtime_dists() const {
    return enable_runtime_dists_ == EnableRuntimeDists::On;
  }

  EnableHaltDists enable_halt_dists_{EnableHaltDists::On};
  EnableRuntimeDists enable_runtime_dists_{EnableRuntimeDists::On};
};

}  // namespace soro::simulation
