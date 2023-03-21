#include "utl/cmd_line_parser.h"
#include "utl/logging.h"

#include "soro/server/soro_server.h"

int main(int argc, char const** argv) {
  using namespace soro::server;

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

  if (!fs::exists(SERVER_RESOURCE_DIR)) {
    uLOG(utl::err) << "cmake should have created "
                   << absolute(SERVER_RESOURCE_DIR)
                   << ", but it does not exist for binary location "
                   << absolute(fs::current_path());
    return 1;
  }

  if (!fs::exists(COORD_FILE)) {
    uLOG(utl::err) << "cmake should have created " << fs::absolute(COORD_FILE)
                   << ", but it does not exist for binary location "
                   << fs::absolute(fs::current_path());
    return 1;
  }

  if (!fs::exists(PROFILE_FILE)) {
    uLOG(utl::err) << "cmake should have created " << fs::absolute(PROFILE_FILE)
                   << ", but it does not exist for binary location "
                   << fs::absolute(fs::current_path());
    return 1;
  }

  fs::create_directory(SERVER_INFRA_DIR);
  fs::create_directory(SERVER_TIMTEABLE_DIR);
  fs::create_directory(SERVER_TILES_DIR);
  fs::create_directory(SERVER_OSM_DIR);
  fs::create_directory(SERVER_TMP_DIR);

  soro_server server(s);
  server.run(s);
}
