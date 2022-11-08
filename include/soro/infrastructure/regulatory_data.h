#pragma once

#include "soro/base/soro_types.h"
#include "soro/utls/file/loaded_file.h"

namespace soro::infra {

struct regulatory_station_data {
  soro::map<soro::string, soro::string> ds100_to_full_name_{};
};

struct regulatory_line_data {};

regulatory_station_data parse_regulatory_stations(
    std::vector<utls::loaded_file> const& regulatory_station_files);

regulatory_line_data parse_regulatory_line_data(
    std::vector<utls::loaded_file> const&);

}  // namespace soro::infra