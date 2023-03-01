#include "soro/server/soro_server.h"

#include "net/stop_handler.h"
#include "net/web_server/responses.h"
#include "net/web_server/serve_static.h"
#include "net/web_server/web_server.h"

#include "utl/logging.h"

#include "soro/base/time.h"

namespace soro::server {

using namespace net;

void soro_server::set_up_routes() {
  router_.reply_hook([](web_server::http_res_t const& resp) {
    std::visit(
        [](auto&& r) {
          if (r.result() != boost::beast::http::status::ok) {
            uLOG(utl::info) << "bad response status: " << r.result();
          }
        },
        resp);
  });

  // 0.0.0.0:8080/infrastructure/
  router_.route("GET", R"(/infrastructure/$)",
                [&](net::query_router::route_request const& req,
                    web_server::http_res_cb_t const& cb, bool const) {
                  cb(infra_state_.serve_infrastructure_names(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/tiles/{int}/{int}/{int}.mvt
  router_.route(
      "GET",
      R"(/infrastructure\/([a-zA-Z0-9_-]+)\/tiles\/(\d+)\/(\d+)\/(\d+).mvt)",
      [&](net::query_router::route_request const& req,
          web_server::http_res_cb_t const& cb,
          bool const) { cb(tiles_.serve_tile(req)); });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/stations/
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/stations/$)",
                [&](net::query_router::route_request const& req,
                    web_server::http_res_cb_t const& cb,
                    bool const) { cb(infra_state_.serve_station_names(req)); });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/stations/{id}
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/stations/(\d+)$)",
                [&](net::query_router::route_request const& req,
                    web_server::http_res_cb_t const& cb,
                    bool const) { cb(infra_state_.serve_station(req)); });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/station_routes/{id}
  router_.route("GET",
                R"(/infrastructure\/([a-zA-Z0-9_-]+)\/station_routes/(\d+)$)",
                [&](net::query_router::route_request const& req,
                    web_server::http_res_cb_t const& cb,
                    bool const) { cb(infra_state_.serve_station_route(req)); });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/interlocking_routes/{id}
  router_.route(
      "GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/interlocking_routes/(\d+)$)",
      [&](net::query_router::route_request const& req,
          web_server::http_res_cb_t const& cb,
          bool const) { cb(infra_state_.serve_interlocking_route(req)); });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/exclusion_sets/
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/exclusion_sets/$)",
                [&](net::query_router::route_request const& req,
                    web_server::http_res_cb_t const& cb, bool const) {
                  cb(infra_state_.serve_exclusion_sets(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/exclusion_sets/{id}
  router_.route("GET",
                R"(/infrastructure\/([a-zA-Z0-9_-]+)\/exclusion_sets/(\d+)$)",
                [&](net::query_router::route_request const& req,
                    web_server::http_res_cb_t const& cb,
                    bool const) { cb(infra_state_.serve_exclusion_set(req)); });

  // if nothing matches: match all and try to serve static file
  router_.route("GET", ".*",
                [&](net::query_router::route_request const& req,
                    web_server::http_res_cb_t const& cb, bool const) {
                  auto const success = net::serve_static_file(
                      SERVER_RESOURCE_DIR.string(), req, cb);
                  if (!success) {
                    cb(net::not_found_response(req));
                  }
                });
}

soro_server::soro_server(server_settings const& settings) {
  infra_state_ = get_infra_state(settings);
  tiles_ = get_tile_module(settings, infra_state_);

  boost::asio::io_context ioc;
  net::web_server serv{ioc};

  set_up_routes();
  serv.on_http_request([&](web_server::http_req_t const& rq,
                           web_server::http_res_cb_t const& cb,
                           bool const ssl) {
    uLOG(utl::info) << "received request: " << rq.target();
    router_(rq, cb, ssl);
  });

  boost::system::error_code ec;
  serv.init(settings.address_.val(), settings.port_.val(), ec);

  if (ec) {
    uLOG(utl::err) << "init error: " << ec.message();
    std::abort();
  }

  stop_handler const stop(ioc, [&]() {
    serv.stop();
    ioc.stop();
  });

  uLOG(utl::info) << "soro-server running on " << settings.address_.val() << ":"
                  << settings.port_.val();

  serv.run();

  if (settings.test_.val()) {
    ioc.run_for(seconds{1});
  } else {
    ioc.run();
  }
}

}  // namespace soro::server