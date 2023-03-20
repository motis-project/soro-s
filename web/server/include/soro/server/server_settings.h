#pragma once

#include "utl/cmd_line_parser.h"

namespace fs = std::filesystem;

namespace soro::server {

// top level directory for all server resources, should be created by cmake
const fs::path SERVER_RESOURCE_DIR("server_resources/");
// DS100 -> GPS mapping
const fs::path COORD_FILE = SERVER_RESOURCE_DIR / "misc" / "btrs_geo.csv";
// OSM profile.lua
const fs::path PROFILE_FILE = SERVER_RESOURCE_DIR / "profile" / "profile.lua";
// put serialized infrastructure in this directory
const fs::path SERVER_INFRA_DIR = SERVER_RESOURCE_DIR / "infrastructure";
// put serialized timetables in this directory
const fs::path SERVER_TIMTEABLE_DIR = SERVER_RESOURCE_DIR / "timetable";
// tiles.mdb in this directory
const fs::path SERVER_TILES_DIR = SERVER_RESOURCE_DIR / "tiles";
// infrastructure.osm in this directory
const fs::path SERVER_OSM_DIR = SERVER_RESOURCE_DIR / "osm";
// temporary files in this directory
const fs::path SERVER_TMP_DIR = SERVER_RESOURCE_DIR / "tmp";

struct server_settings {
  utl::cmd_line_flag<
      fs::path, UTL_LONG("--infrastructure_dir"),
      UTL_DESC("where the server reads the infrastructure resources from")>
      infra_dir_{"../../resources/infrastructure/"};

  utl::cmd_line_flag<fs::path, UTL_LONG("--timetable_dir"),
                     UTL_DESC(
                         "where the server reads the timetable resources from")>
      timetable_dir_{"../../resources/timetable/"};

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