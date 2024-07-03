#pragma once

#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::infra {

soro::vector_map<station::id, coordinates> parse_station_coordinates(
    iss_files const& iss_files, infrastructure_t const& infra);

}
