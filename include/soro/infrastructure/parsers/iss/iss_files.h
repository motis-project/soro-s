#pragma once

#include <filesystem>

#include "soro/utls/file/loaded_file.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

struct iss_files {
  std::vector<utls::loaded_file> rail_plan_files_;
  std::vector<utls::loaded_file> core_data_files_;
  std::vector<utls::loaded_file> regulatory_station_files_;
  std::vector<utls::loaded_file> regulatory_line_files_;
};

iss_files get_iss_files(std::filesystem::path const& fp);

}  // namespace soro::infra
