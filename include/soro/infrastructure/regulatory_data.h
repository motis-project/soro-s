#pragma once

#include "soro/base/soro_types.h"
#include "soro/infrastructure/line.h"
#include "soro/utls/file/loaded_file.h"

namespace soro::infra {

struct regulatory_station_data {
  soro::map<soro::string, soro::string> ds100_to_full_name_{};
};

regulatory_station_data parse_regulatory_stations(
    std::vector<utls::loaded_file> const& regulatory_station_files);

soro::map<line::id, line> parse_lines(
    std::vector<utls::loaded_file> const& regulatory_line_files);

}  // namespace soro::infra