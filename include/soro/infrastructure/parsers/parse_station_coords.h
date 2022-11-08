#pragma once

#include <filesystem>

#include "soro/utls/coordinates/gps.h"

#include "soro/base/soro_types.h"

#include "soro/infrastructure/station/station.h"

namespace soro::infra {

soro::vector<utls::gps> parse_station_coords(
    std::filesystem::path const& gps_path,
    soro::map<soro::string, station::ptr> const& ds100_to_station);

}  // namespace soro::infra