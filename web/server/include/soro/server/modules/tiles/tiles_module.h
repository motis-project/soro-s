#pragma once

#include <filesystem>

#include "tiles/get_tile.h"

#include "net/web_server/query_router.h"
#include "net/web_server/web_server.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"

namespace fs = std::filesystem;

namespace soro::server {

struct tiles_module {
  struct context {
    explicit context(std::filesystem::path const& tile_db_path)
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

  net::web_server::string_res_t serve_tile(
      net::query_router::route_request const& req);

  std::unordered_map<std::string_view, context> tile_contexts_;
};

tiles_module get_tile_module(server_settings const& settings,
                             infrastructure_module const& infra_state);

}  // namespace soro::server
