#include "soro/server/soro_server.h"

#include <cstdlib>
#include <variant>

#include "boost/beast/http/status.hpp"

#include "boost/asio/io_context.hpp"

#include "net/stop_handler.h"
#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/serve_static.h"
#include "net/web_server/web_server.h"

#include "utl/logging.h"

#include "soro/base/time.h"

#include "soro/server/server_settings.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"
#include "soro/server/modules/ordering/ordering_module.h"
#include "soro/server/modules/search/search_module.h"
#include "soro/server/modules/tiles/tiles_module.h"
#include "soro/server/modules/timetable/timetable_module.h"

namespace soro::server {

using namespace net;

void soro_server::set_up_routes(server_settings const& s) {
  router_.reply_hook([](web_server::http_res_t const& resp) {
    std::visit(
        [](auto&& r) {
          if (r.result() != boost::beast::http::status::ok) {
            uLOG(utl::info) << "bad response status: " << r.result();
          }
        },
        resp);
  });

  // 0.0.0.0:8080/infrastructures/
  router_.route("GET", R"(/infrastructures\/?$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_infrastructure_names(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/tiles/{int}/{int}/{int}.mvt
  router_.route(
      "GET",
      R"(/infrastructure\/([a-zA-Z0-9_-]+)\/tiles\/(\d+)\/(\d+)\/(\d+).mvt)",
      [this](net::query_router::route_request const& req,
             web_server::http_res_cb_t const& cb,
             bool const) { cb(tiles_module_.serve_tile(req)); });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/bounding_box/
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/bounding_box\/?$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_bounding_box(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/stations/
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/stations/$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_station_names(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/station/{id}
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/station/(\d+)$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_station(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/station_route/{id}
  router_.route("GET",
                R"(/infrastructure\/([a-zA-Z0-9_-]+)\/station_route/(\d+)$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_station_route(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/interlocking_route/{id}
  router_.route(
      "GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/interlocking_route/(\d+)$)",
      [this](net::query_router::route_request const& req,
             web_server::http_res_cb_t const& cb, bool const) {
        cb(infrastructure_module_.serve_interlocking_route(req));
      });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/exclusion_sets/{id}
  router_.route("GET",
                R"(/infrastructure\/([a-zA-Z0-9_-]+)\/exclusion_sets/(\d+)$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_exclusion_set(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/element/{id}
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/element/(\d+)$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_element(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/node/{id}
  router_.route("GET", R"(/infrastructure\/([a-zA-Z0-9_-]+)\/node/(\d+)$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(infrastructure_module_.serve_node(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/search/{search_string}
  router_.route("GET",
                R"(/infrastructure/([a-zA-Z0-9_-]+)/search/([a-zA-Z0-9_-]+)$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(search_module_.serve_search(req, infrastructure_module_));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/timetables/
  router_.route("GET", R"(/infrastructure/([a-zA-Z0-9_-]+)/timetables\/?$)",
                [this](net::query_router::route_request const& req,
                       web_server::http_res_cb_t const& cb, bool const) {
                  cb(timetable_module_.serve_timetable_names(req));
                });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/timetable/{timetable_name}
  router_.route(
      "GET",
      R"(/infrastructure/([a-zA-Z0-9_-]+)/timetable/([a-zA-Z0-9_-]+)\/?$)",
      [this](net::query_router::route_request const& req,
             web_server::http_res_cb_t const& cb,
             bool const) { cb(timetable_module_.serve_timetable(req)); });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/timetable/{timetable_name}/train/{train_id}
  router_.route(
      "GET",
      R"(/infrastructure/([a-zA-Z0-9_-]+)/timetable/([a-zA-Z0-9_-]+)\/train/([0-9])\/?$)",
      [this](net::query_router::route_request const& req,
             web_server::http_res_cb_t const& cb, bool const) {
        cb(timetable_module_.serve_intervals(req, infrastructure_module_));
      });

  // 0.0.0.0:8080/infrastructure/{infrastructure_name}/timetable/{timetable_name}/ordering?from={from}&to={to}&trainIds={trainId,trainId,...}
  router_.route(
      "GET",
      R"(/infrastructure/([a-zA-Z0-9_-]+)/timetable/([a-zA-Z0-9_-]+)/ordering\?from=([0-9]+)&to=([0-9]+)&trainIds=(\d*(?:,\d+)*)$)",
      [this](net::query_router::route_request const& req,
             web_server::http_res_cb_t const& cb, bool const) {
        cb(ordering_module_.serve_ordering_graph(req, infrastructure_module_,
                                                 timetable_module_));
      });

  // if nothing matches: match all and try to serve static file
  router_.route("GET", ".*",
                [&s](net::query_router::route_request const& req,
                     web_server::http_res_cb_t const& cb, bool const) {
                  auto const success = net::serve_static_file(
                      s.server_resource_dir_.val().string(), req, cb);
                  if (!success) {
                    cb(net::not_found_response(req));
                  }
                });
}

soro_server::soro_server(server_settings const& s)
    : infrastructure_module_{get_infrastructure_module(s)},
      tiles_module_{get_tiles_module(s, infrastructure_module_)},
      search_module_{get_search_module(infrastructure_module_)},
      timetable_module_{get_timetable_module(s, infrastructure_module_)},
      ordering_module_{get_ordering_module()} {
  set_up_routes(s);
}

void soro_server::run(server_settings const& s) {
  boost::asio::io_context ioc;
  net::web_server serv{ioc};

  serv.on_http_request([this](web_server::http_req_t const& rq,
                              web_server::http_res_cb_t const& cb,
                              bool const ssl) {
    uLOG(utl::info) << "received request: " << rq.target();
    router_(rq, cb, ssl);
  });

  boost::system::error_code ec;  // NOLINT
  serv.init(s.address_.val(), s.port_.val(), ec);

  if (ec) {
    uLOG(utl::err) << "init error: " << ec.message();
    std::abort();
  }

  stop_handler const stop(ioc, [&]() {
    serv.stop();
    ioc.stop();
  });

  uLOG(utl::info) << "soro-server running on " << s.address_.val() << ":"
                  << s.port_.val();

  serv.run();

  if (s.test_.val()) {
    ioc.run_for(seconds{1});
  } else {
    ioc.run();
  }
}

}  // namespace soro::server