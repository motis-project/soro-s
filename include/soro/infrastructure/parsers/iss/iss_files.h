#pragma once

#include <filesystem>
#include <vector>

#include "pugixml.hpp"

namespace soro::infra {

struct iss_files {
  explicit iss_files(std::filesystem::path const& fp);

  pugi::xml_document index_;
  std::vector<pugi::xml_document> rail_plan_files_;
  std::vector<pugi::xml_document> core_data_files_;
  std::vector<pugi::xml_document> regulatory_station_files_;
  std::vector<pugi::xml_document> regulatory_line_files_;
  std::vector<pugi::xml_document> construction_work_files_;
};

}  // namespace soro::infra
