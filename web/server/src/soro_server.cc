#include "soro/server/soro_server.h"

#include <chrono>

#include "utl/logging.h"
#include "utl/parser/mmap_reader.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "tiles/get_tile.h"
#include "tiles/parse_tile_url.h"
#include "tiles/perf_counter.h"
#include "tiles/util.h"

#include "soro/server/http_server.h"

namespace soro::server {

std::string url_decode(request_t const& req) {
  auto const& in = req.target();

  if (in == "/") {
    return "/index.html";
  }

  std::string out;
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      utl::verify(i + 3 <= in.size(), "invalid url");
      int value = 0;
      std::istringstream is{std::string{in.substr(i + 1, 2)}};
      utl::verify(static_cast<bool>(is >> std::hex >> value), "invalid url");
      out += static_cast<char>(value);
      i += 2;
    } else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }
  return out;
}

bool is_sane(std::string const& url) {
  return url.find("..") == std::string::npos;
}

void serve_not_found(response_t& res) {
  uLOG(utl::info) << "Returned: 404 - Not found.";
  res.result(http::status::not_found);
}

void set_content_type(response_t& res, fs::path const& served_file) {
  if (served_file.extension() == ".html") {
    res.set(http::field::content_type, "text/html");
  } else if (served_file.extension() == ".css") {
    res.set(http::field::content_type, "text/css");
  } else if (served_file.extension() == ".js") {
    res.set(http::field::content_type, "text/javascript");
  } else if (served_file.extension() == ".png") {
    res.set(http::field::content_type, "image/png");
  } else if (served_file.extension() == ".svg") {
    res.set(http::field::content_type, "image/svg+xml");
  } else if (served_file.extension() == ".wasm") {
    res.set(http::field::content_type, "application/wasm");
  }
}

void serve_dir(fs::path const& root_dir, fs::path const& rel_dir,
               response_t& res) {
  namespace rj = rapidjson;

  rj::Document dir_doc(rj::kObjectType);
  auto& allocator = dir_doc.GetAllocator();

  rj::Value dirs(rj::kArrayType);
  rj::Value files(rj::kArrayType);

  rj::Value pp_v;
  auto const pp = rel_dir.string();
  pp_v.SetString(pp.c_str(), static_cast<rj::SizeType>(pp.size()), allocator);
  dir_doc.AddMember("parent_path", pp_v, allocator);

  for (auto const& dir_entry : fs::directory_iterator{root_dir / rel_dir}) {
    rj::Value entry;
    auto const fn = dir_entry.path().filename().string();
    entry.SetString(fn.c_str(), static_cast<rj::SizeType>(fn.size()),
                    allocator);

    if (dir_entry.is_directory()) {
      dirs.PushBack(entry, allocator);
    } else {
      files.PushBack(entry, allocator);
    }
  }

  dir_doc.AddMember("files", files, allocator);
  dir_doc.AddMember("dirs", dirs, allocator);

  rj::StringBuffer buf;
  rj::Writer<rj::StringBuffer> writer(buf);
  dir_doc.Accept(writer);
  res.body() = buf.GetString();

  res.result(http::status::ok);
  res.set(http::field::content_type, "application/json");
}

void serve_file(std::string const& decoded_url,
                fs::path const& server_resource_dir, response_t& res) {
  if (server_resource_dir.empty()) {
    return serve_not_found(res);
  }

  auto p = server_resource_dir / decoded_url.substr(1);
  if (!fs::exists(p)) {
    return serve_not_found(res);
  }

  if (!fs::is_directory(p)) {
    utl::mmap_reader const mem{p.string().c_str()};
    res.body() = std::string{mem.m_.ptr(), mem.m_.size()};
    res.result(http::status::ok);
    set_content_type(res, p);
  } else {
    return serve_dir(server_resource_dir, fs::path{decoded_url.substr(1)}, res);
  }
}

bool serve_tile(server::serve_context& sc, std::string const& decoded_url,
                auto const& req, auto& res) {
  static tiles::regex_matcher const matcher{R"(\/(\d+)\/(\d+)\/(\d+).mvt)"};
  auto const match = matcher.match(decoded_url);
  if (!match) {
    return false;
  }

  if (req[http::field::accept_encoding]  //
          .find("deflate") == std::string_view::npos) {
    res.result(http::status::not_implemented);
    return true;
  }

  auto const tile = tiles::url_match_to_tile(*match);

  tiles::perf_counter pc;
  auto rendered_tile =
      get_tile(sc.tile_handle_, sc.pack_handle_, sc.render_ctx_, tile, pc);
  perf_report_get_tile(pc);

  if (rendered_tile) {
    res.body() = std::move(*rendered_tile);
    res.set(http::field::content_encoding, "deflate");
    res.result(http::status::ok);
  } else {
    res.result(http::status::no_content);
  }
  return true;
}

void initialize_serve_contexts(server::serve_contexts& contexts,
                               fs::path const& server_resource_dir) {
  fs::path const infra_dir = server_resource_dir / "infrastructure";
  for (auto const& dir_entry : fs::directory_iterator{infra_dir}) {
    auto const filename = dir_entry.path().filename();

    auto tile_db_name = filename;
    tile_db_name.replace_extension(".mdb");
    auto const tile_db_path = dir_entry.path() / "tiles" / tile_db_name;

    contexts.try_emplace(filename.string(), tile_db_path);
  }
}

server::server(std::string const& address, port_t const port,
               fs::path const& server_resource_dir, bool const test) {
  initialize_serve_contexts(contexts_, server_resource_dir);

  serve_forever(
      address, port,
      [&](auto const& req, auto& res) {
        std::string const url = url_decode(req);

        uLOG(utl::info) << "Request: " << url;

        if (!is_sane(url)) {
          res.result(http::status::forbidden);
          return;
        }

        auto const tiles_pos = url.find("/tiles/");
        bool const should_serve_tiles = tiles_pos != std::string::npos;

        switch (req.method()) {
          case http::verb::options: {
            res.result(http::status::no_content);
            break;
          }

          case http::verb::get:
          case http::verb::head: {
            if (should_serve_tiles) {
              auto const infra_name = url.substr(1, url.find('/', 1) - 1);
              auto sc_it = contexts_.find(infra_name);
              if (sc_it == std::cend(contexts_)) {
                return serve_not_found(res);
              }

              serve_tile(sc_it->second, url.substr(tiles_pos + 6), req, res);
            } else {
              serve_file(url, server_resource_dir, res);
            }
            break;
          }

          default: {
            res.result(http::status::method_not_allowed);
          }
        }

        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_headers,
                "X-Requested-With, Content-Type, Accept, Authorization");
        res.set(http::field::access_control_allow_methods,
                "GET, POST, PUT, DELETE, OPTIONS, HEAD");
        std::string const csp =
            "default-src 'self' ;"
            "script-src 'unsafe-eval' 'unsafe-inline' 'self';"
            "worker-src blob: ;"
            "style-src 'self' 'unsafe-inline';"
            "style-src-elem 'unsafe-inline' 'self';"
            "img-src 'self' data: blob: ;";
        res.set("Content-Security-Policy-Report-Only", csp);
        res.set("Cross-Origin-Embedder-Policy", "require-corp");
        res.set("Cross-Origin-Opener-Policy", "same-origin");
      },
      test);
}

void server::serve_forever(std::string const& address, port_t const port,
                           callback_t&& cb, bool const test) {
  using namespace std::chrono_literals;

  try {
    net::io_context ioc{static_cast<int>(std::thread::hardware_concurrency())};
    tcp::acceptor acceptor{ioc, {net::ip::make_address(address), port}};
    tcp::socket socket{ioc};

    http_server(acceptor, socket, cb);

    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&](boost::system::error_code const&, int) { ioc.stop(); });

    auto const worker = [&ioc, &test]() { test ? ioc.run_for(1s) : ioc.run(); };

    std::vector<std::thread> threads;
    for (auto i = 1ULL; i < std::thread::hardware_concurrency(); ++i) {
      threads.emplace_back(worker);
    }

    uLOG(utl::info) << "tiles-server started on " << address << ":" << port;
    worker();

    std::for_each(begin(threads), end(threads), [](auto& t) { t.join(); });
  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

}  // namespace soro::server
