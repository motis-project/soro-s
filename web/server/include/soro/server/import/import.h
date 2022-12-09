#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace soro::server {

namespace fs = std::filesystem;

struct import_settings {
  import_settings(fs::path osm_path, fs::path db_path, fs::path tmp_dir)
      : osm_path_{std::move(osm_path)},
        db_path_{std::move(db_path)},
        tmp_dir_{std::move(tmp_dir)},
        osm_profile_{"server_resources/profile/profile.lua"} {}

  fs::path osm_path_;
  fs::path db_path_;
  fs::path tmp_dir_;

  fs::path osm_profile_;
};

void import_tiles(import_settings const& settings);

}  // namespace soro::server
