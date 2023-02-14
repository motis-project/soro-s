#pragma once

#include <filesystem>
#include <vector>

#include "tiles/db/pack_file.h"
#include "tiles/db/tile_database.h"
#include "tiles/get_tile.h"
#include "tiles/parse_tile_url.h"
#include "tiles/perf_counter.h"
#include "tiles/util.h"

#include "soro/server/http_server.h"
#include "soro/server/osm_util.h"

namespace soro::server {

namespace fs = std::filesystem;

using port_t = uint16_t;

struct server {
  struct serve_context {
    explicit serve_context(fs::path const& tile_db_path)
        : db_env_{tiles::make_tile_database(tile_db_path.string().c_str(),
                                            tiles::kDefaultSize)},
          tile_handle_{db_env_},
          render_ctx_{tiles::make_render_ctx(tile_handle_)},
          pack_handle_{tile_db_path.string().c_str()} {}

    lmdb::env db_env_;
    tiles::tile_db_handle tile_handle_;
    tiles::render_ctx render_ctx_;
    tiles::pack_handle pack_handle_;
  };

  using serve_contexts = std::unordered_map<std::string, serve_context>;

  server(std::string const& address, port_t const port,
         fs::path const& server_resource_dir, bool const test,
         const std::unordered_map<std::string, std::vector<soro::server::osm_halt>>& osm_halts);

  static void serve_forever(std::string const& address, port_t const port,
                            callback_t&& cb, bool const test);

  serve_contexts contexts_;
};

}  // namespace soro::server
