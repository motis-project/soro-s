#include "utl/cmd_line_parser.h"
#include "utl/logging.h"

#include "soro/server/soro_server.h"

namespace fs = std::filesystem;

using namespace soro::server;

int main(int argc, char const** argv) {
  std::cout << "\n\t\t[SORO Server]\n" << std::endl;

  server_settings s;

  try {
    s = utl::parse<server_settings>(argc, argv);
  } catch (std::exception const& e) {
    uLOG(utl::err) << "options error: " << e.what() << "\n";
    uLOG(utl::err) << utl::description<server_settings>() << "\n";
    uLOG(utl::err) << utl::print_flags(s) << "\n";
    return 1;
  }

  if (s.help_) {
    std::cout << utl::description<server_settings>() << std::endl;
    return 0;
  }

  if (!fs::exists(s.server_resource_dir_.val())) {
    uLOG(utl::err) << "please specify a valid server_resource_dir, "
                   << s.server_resource_dir_.val() << " does not exist";
    return 1;
  }

  if (!fs::exists(s.coord_file())) {
    uLOG(utl::err) << "cmake should have created " << s.coord_file()
                   << ", but it does not exist";
    return 1;
  }

  if (!fs::exists(s.profile_file())) {
    uLOG(utl::err) << "cmake should have created " << s.profile_file()
                   << ", but it does not exist";
    return 1;
  }

  if (!fs::exists(s.infrastructure_sources_.val()) &&
      fs::is_directory(s.infrastructure_sources_.val())) {
    uLOG(utl::err) << "please specify a valid infrastructure source directory, "
                   << s.infrastructure_sources_.val() << " does not exist";
    return 1;
  }

  if (!fs::exists(s.timetable_sources_.val()) &&
      fs::is_directory(s.timetable_sources_.val())) {
    uLOG(utl::err) << "please specify a valid timetable source directory, "
                   << s.timetable_sources_.val() << " does not exist";
    return 1;
  }

  fs::create_directory(s.server_infra_dir());
  fs::create_directory(s.server_timetable_dir());
  fs::create_directory(s.tiles_dir());
  fs::create_directory(s.osm_dir());
  fs::create_directory(s.tmp_dir());

  soro_server server(s);
  server.run(s);
}
