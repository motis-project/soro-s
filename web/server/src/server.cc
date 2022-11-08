#include <filesystem>

#include "utl/cmd_line_parser.h"
#include "utl/to_vec.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/server/import/import.h"
#include "soro/server/osm_export/osm_export.h"
#include "soro/server/soro_server.h"

namespace fs = std::filesystem;

struct server_settings {
  utl::cmd_line_flag<fs::path, UTL_LONG("--resource_dir"),
                     UTL_DESC("where the server reads the resources from")>
      resource_dir_{"../../resources/"};

  utl::cmd_line_flag<fs::path, UTL_LONG("--server_resource_dir"),
                     UTL_DESC("where the server puts the generated resources")>
      server_resource_dir_{"server_resources/"};

  utl::cmd_line_flag<std::string, UTL_LONG("--address"),
                     UTL_DESC("ip address the server listens on")>
      address_{"0.0.0.0"};

  utl::cmd_line_flag<soro::server::port_t, UTL_LONG("--port"),
                     UTL_DESC("port the server listens on")>
      port_{8080};

  utl::cmd_line_flag<bool, UTL_LONG("--regenerate"), UTL_SHORT("-r"),
                     UTL_DESC("regenerate server resources")>
      regenerate_{false};

  utl::cmd_line_flag<bool, UTL_LONG("--test"), UTL_SHORT("-t"),
                     UTL_DESC("start in test mode - quit after 1s")>
      test_{false};
};

bool is_infrastructure_file(fs::path const& possible_infrastructure) {
  return !fs::is_directory(possible_infrastructure) &&
         possible_infrastructure.has_extension() &&
         possible_infrastructure.extension() == ".iss";
}

void exists_or_create_dir(fs::path const& dir_path) {
  if (!fs::exists(dir_path)) {
    fs::create_directory(dir_path);
  }
}

auto set_up_infrastructure(fs::path const& from_dir, fs::path const& to_dir) {
  std::vector<fs::path> new_infrastructure_files;

  for (auto const& dir_entry : fs::directory_iterator{from_dir}) {
    if (!is_infrastructure_file(dir_entry)) {
      continue;
    }

    auto const& from_file = dir_entry.path();
    auto const to_file = to_dir / from_file.filename();

    bool const new_file =
        !fs::exists(to_file) || soro::utls::load_file(from_file).hash() !=
                                    soro::utls::load_file(to_file).hash();

    if (new_file) {
      fs::copy(from_file, to_file, fs::copy_options::overwrite_existing);

      new_infrastructure_files.push_back(to_file);
    }
  }

  return new_infrastructure_files;
}

int failed_startup() { return 1; }

int main(int argc, char const** argv) {
  server_settings s;
  std::cout << "\n\t\t[SORO Server]\n\n";

  try {
    s = utl::parse<server_settings>(argc, argv);
  } catch (std::exception const& e) {
    std::cout << "options error: " << e.what() << "\n";
    return failed_startup();
  }

  auto const coord_file = s.resource_dir_ / "misc" / "btrs_geo.csv";

  fs::path const tt_dir = s.server_resource_dir_ / "timetable";
  fs::path const infra_dir = s.server_resource_dir_ / "infrastructure";

  exists_or_create_dir(s.server_resource_dir_);
  exists_or_create_dir(tt_dir);
  exists_or_create_dir(infra_dir);

  std::vector<fs::path> infra_todo;
  for (auto const& dir_entry :
       fs::directory_iterator{s.resource_dir_ / "infrastructure"}) {

    if (!dir_entry.is_directory()) {
      continue;
    }

    auto const res_path = infra_dir / dir_entry.path().filename();

    if (!fs::exists(res_path) ||
        last_write_time(res_path) < dir_entry.last_write_time()) {
      infra_todo.emplace_back(dir_entry.path());
    }
  }

  if (s.regenerate_) {
    infra_todo.clear();
    for (auto&& dir_entry :
         fs::directory_iterator{s.resource_dir_ / "infrastructure"}) {

      infra_todo.emplace_back(dir_entry.path());
    }
  }

  for (auto const& infra_file : infra_todo) {
    auto const infra_res_dir = infra_dir / infra_file.filename();
    exists_or_create_dir(infra_res_dir);

    soro::infra::infrastructure_options opts;
    opts.infrastructure_path_ = infra_file;
    opts.gps_coord_path_ = coord_file;

    soro::infra::infrastructure infra(opts);

    auto const osm_file =
        infra_res_dir / infra_file.filename().replace_extension(".osm");
    soro::server::osm_export::export_and_write(*infra, osm_file);

    auto const tiles_dir = infra_res_dir / "tiles";
    exists_or_create_dir(tiles_dir);

    auto const tmp_dir = infra_res_dir / "tmp";
    exists_or_create_dir(tmp_dir);

    soro::server::import_settings import_settings(
        osm_file,
        infra_res_dir / "tiles" /
            infra_file.filename().replace_extension(".mdb"),
        tmp_dir);
    soro::server::import_tiles(import_settings);
  }

  soro::server::server server(s.address_.val(), s.port_.val(),
                              s.server_resource_dir_.val(), s.test_);
}