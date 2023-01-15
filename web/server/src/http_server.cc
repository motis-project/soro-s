#include "soro/server/http_server.h"

#include <exception>
#include <memory>
#include <string>

#include "utl/logging.h"

#include "boost/asio.hpp"
#include "boost/beast/http.hpp"

namespace soro::server {

using namespace utl;

void http_connection::start() {
  auto self = shared_from_this();
  http::async_read(
      socket_, buffer_, request_, [self](beast::error_code ec, std::size_t) {
        if (!ec) {
          self->response_.version(self->request_.version());
          self->response_.keep_alive(false);

          try {
            self->callback_(self->request_, self->response_);
          } catch (std::exception const& e) {
            uLOG(err) << "unhandled error: {}" << e.what();
            self->response_.result(http::status::internal_server_error);
          } catch (...) {
            uLOG(err) << "unhandled unknown error";
            self->response_.result(http::status::internal_server_error);
          }
          self->response_.set(http::field::content_length,
                              std::to_string(self->response_.body().size()));
          http::async_write(self->socket_, self->response_,
                            [self](beast::error_code async_ec, std::size_t) {
                              self->socket_.shutdown(tcp::socket::shutdown_send,
                                                     async_ec);
                              self->deadline_.cancel();
                            });
        }
      });
  check_deadline();
}

void http_connection::check_deadline() {
  auto self = shared_from_this();
  deadline_.async_wait([self](beast::error_code ec) {
    if (!ec) {
      self->socket_.close(ec);
    }
  });
}

void http_server(tcp::acceptor& acceptor, tcp::socket& socket,
                 callback_t const& cb) {
  acceptor.async_accept(socket, [&](beast::error_code ec) {
    if (!ec) {
      std::make_shared<http_connection>(std::move(socket), cb)->start();
    }
    http_server(acceptor, socket, cb);
  });
}

}  // namespace soro::server
