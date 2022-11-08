#pragma once

#include <filesystem>
#include <functional>
#include <memory>

#include "boost/asio.hpp"
#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/version.hpp"

namespace beast = boost::beast;  // from "boost/beast.hpp"
namespace http = beast::http;  // from "boost/beast/http.hpp"
namespace net = boost::asio;  // from "boost/asio.hpp"
using tcp = boost::asio::ip::tcp;  // from "boost/asio/ip/tcp.hpp"

namespace soro::server {

using request_t = http::request<http::dynamic_body>;
using response_t = http::response<http::string_body>;
using callback_t = std::function<void(request_t const&, response_t&)>;

struct http_connection : public std::enable_shared_from_this<http_connection> {
  http_connection(tcp::socket socket, callback_t const& callback)
      : socket_{std::move(socket)}, callback_{callback} {}

  void start();
  void check_deadline();

  tcp::socket socket_;
  beast::flat_buffer buffer_{8192};
  request_t request_;
  response_t response_;
  callback_t const& callback_;
  net::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(60)};
};

void http_server(tcp::acceptor& acceptor, tcp::socket& socket,
                 callback_t const& cb);

}  // namespace soro::server
