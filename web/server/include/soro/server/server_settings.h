#pragma once

#include "utl/cmd_line_parser.h"

namespace soro::server {

struct server_settings {
  // DS100 -> GPS mapping
  std::filesystem::path coord_file() const {
    return server_resource_dir_.val() / "misc" / "btrs_geo.csv";
  }

  // OSM profile.lua
  std::filesystem::path profile_file() const {
    return server_resource_dir_.val() / "profile" / "profile.lua";
  }

  // put serialized infrastructure in this directory
  std::filesystem::path server_infra_dir() const {
    return server_resource_dir_.val() / "infrastructure";
  }

  // put serialized timetables in this directory
  std::filesystem::path server_timetable_dir() const {
    return server_resource_dir_.val() / "timetable";
  }

  // tiles.mdb in this directory
  std::filesystem::path tiles_dir() const {
    return server_resource_dir_.val() / "tiles";
  }

  // infrastructure.osm in this directory
  std::filesystem::path osm_dir() const {
    return server_resource_dir_.val() / "osm";
  }

  // temporary files in this directory
  std::filesystem::path tmp_dir() const {
    return server_resource_dir_.val() / "tmp";
  }

  utl::cmd_line_flag<
      std::filesystem::path, UTL_LONG("--server_resource_dir"),
      UTL_DESC("working directory for the server, already created by cmake")>
      server_resource_dir_{"./server_resources/"};

  utl::cmd_line_flag<
      std::filesystem::path, UTL_LONG("--infrastructure_sources"),
      UTL_DESC("where the server reads the infrastructure resources from")>
      infrastructure_sources_{"../../resources/infrastructure/"};

  utl::cmd_line_flag<std::filesystem::path, UTL_LONG("--timetable_sources"),
                     UTL_DESC(
                         "where the server reads the timetable resources from")>
      timetable_sources_{"../../resources/timetable/"};

  utl::cmd_line_flag<std::string, UTL_LONG("--address"),
                     UTL_DESC("ip address the server listens on")>
      address_{"0.0.0.0"};

  utl::cmd_line_flag<std::string, UTL_LONG("--port"),
                     UTL_DESC("port the server listens on")>
      port_{"8080"};

  utl::cmd_line_flag<bool, UTL_LONG("--regenerate"), UTL_SHORT("-r"),
                     UTL_DESC("regenerate server resources")>
      regenerate_{false};

  utl::cmd_line_flag<bool, UTL_LONG("--test"), UTL_SHORT("-t"),
                     UTL_DESC("start in test mode - quit after 1s")>
      test_{false};

  utl::cmd_line_flag<bool, UTL_LONG("--help"), UTL_SHORT("-h"),
                     UTL_DESC("print help information")>
      help_{false};
};

}  // namespace soro::server