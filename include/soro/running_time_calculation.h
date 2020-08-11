#pragma once

#include <string>

namespace soro {

struct train_physics;

float time_until_analytical(train_physics const&, float target_speed_kmh);
float time_until_numerical(train_physics const&, float target_speed_kmh);

std::string compute_running_time(train_physics const&);

}  // namespace soro
