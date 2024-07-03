#pragma once

#include <filesystem>

#include "soro/utls/coordinates/gps.h"

#include "soro/base/soro_types.h"

#include "soro/infrastructure/station/station.h"

namespace soro::server {

soro::map<infra::station::ds100, utls::gps> parse_station_positions(
    std::filesystem::path const& station_positions_csv);

}  // namespace soro::server