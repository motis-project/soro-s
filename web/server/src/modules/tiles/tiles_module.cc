#include "soro/server/modules/tiles/tiles_module.h"

#include <filesystem>
#include <iterator>
#include <string_view>
#include <utility>

#include "boost/beast/http/field.hpp"
#include "boost/beast/http/status.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "tiles/get_tile.h"
#include "tiles/parse_tile_url.h"
#include "tiles/perf_counter.h"

#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/utls/sassert.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/server/server_settings.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "soro/server/modules/tiles/import/import.h"
#include "soro/server/modules/tiles/osm_export/osm_export.h"

namespace soro::server {

using namespace soro::infra;

net::web_server::string_res_t tiles_module::serve_tile(
    net::query_router::route_request const& req) {

  auto const sc_it = tile_contexts_.find(req.path_params_.front());
  if (sc_it == std::end(tile_contexts_)) {
    uLOG(utl::err) << "could not find " << req.path_params_.front()
                   << " infrastructure while serving tiles";
    return net::not_found_response(req);
  }
  auto& sc = sc_it->second;

  net::web_server::string_res_t response;

  if (req[boost::beast::http::field::accept_encoding].find("deflate") ==
      std::string_view::npos) {
    response.result(boost::beast::http::status::not_implemented);
    return response;
  }

  auto const tile = ::tiles::url_match_to_tile(req.path_params_);

  ::tiles::perf_counter pc;
  auto rendered_tile = ::tiles::get_tile(sc.tile_handle_, sc.pack_handle_,
                                         sc.render_ctx_, tile, pc);

  if (rendered_tile) {
    response.body() = std::move(*rendered_tile);
    response.set(boost::beast::http::field::content_encoding, "deflate");
    response.result(boost::beast::http::status::ok);
  } else {
    response.result(boost::beast::http::status::no_content);
  }
  return response;
}

fs::path create_osm_file(server_settings const& settings,
                         infrastructure_module::context const& context) {
  auto osm_path = (settings.osm_dir() /
                   static_cast<std::string_view>(context.infra_->source_))
                      .replace_extension(".osm");
  auto osm_path2 = (settings.osm_dir() /
                    static_cast<std::string_view>(context.infra_->source_))
                       .replace_extension(".osm2");

  if (settings.regenerate_.val() || !fs::exists(osm_path)) {
    {
      utl::scoped_timer const timer("creating osm file");
      osm_export::export_and_write(context.infra_, context.positions_,
                                   osm_path2);
    }

    export_and_write2(context.infra_, context.positions_, osm_path);
  }

  return osm_path;
}

fs::path create_tiles_db(server_settings const& s, infrastructure const& infra,
                         fs::path const& osm_path) {
  auto tile_db_path =
      (s.tiles_dir() / static_cast<std::string_view>(infra->source_))
          .replace_extension(".mdb");

  if (s.regenerate_.val() || !fs::exists(tile_db_path) ||
      last_write_time(osm_path) > last_write_time(tile_db_path)) {
    import_settings const import_settings(osm_path, tile_db_path, s.tmp_dir(),
                                          s.profile_file());
    import_tiles(import_settings);
  }

  return tile_db_path;
}

tiles_module get_tiles_module(server_settings const& settings,
                              infrastructure_module const& infra_m) {
  utl::scoped_timer const timer("creating tiles module");

  tiles_module result;

  for (auto const& context : infra_m.all()) {
    auto const osm_path = create_osm_file(settings, context);
    auto const tile_db_path =
        create_tiles_db(settings, context.infra_, osm_path);

    auto [_, success] = result.tile_contexts_.try_emplace(
        context.infra_->source_, tile_db_path);

    utls::sassert(success,
                  "naming conflict during tile db creation with name {}",
                  context.infra_->source_);
  }

  return result;
}

}  // namespace soro::server
