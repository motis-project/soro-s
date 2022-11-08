#pragma once

#include <filesystem>
#include <vector>

#include "layout.h"
#include "soro/base/soro_types.h"
#include "soro/utls/coordinates/gps.h"

namespace soro::layout {

std::pair<soro::vector<utls::gps>, soro::vector<utls::gps>> layout_to_gps(
    layout const& layout, soro::vector<infra::station::ptr> const& stations,
    soro::vector<utls::gps> const& station_coords);

}  // namespace soro::layout
