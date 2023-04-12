#pragma once

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"

#include "boost/beast/version.hpp"

#include "utl/logging.h"

#include "soro/utls/sassert.h"

namespace soro::server {

struct json_archive {
  json_archive() : archive_{new cereal::JSONOutputArchive(os_)} {}

  json_archive(json_archive const&) = delete;
  auto& operator=(json_archive const&) = delete;

  json_archive(json_archive&&) = delete;
  auto& operator=(json_archive&&) = delete;

  ~json_archive() {
    if (archive_ != nullptr) {
      uLOG(utl::warn) << "Deleting archive without finalizing";
      delete archive_;
    }
  }

  // TODO(julian) when libc++ gets os_.view(), change this to a string_view
  std::string json() {
    if (archive_ != nullptr) {
      this->finalize();
    }

    return os_.str();
  }

  auto& add() const {
    if (archive_ == nullptr) {
      throw utl::fail("don't call add() on finalized archive");
    }

    return *archive_;
  }

  void finalize() {
    utls::sassert(archive_ != nullptr, "double free");

    if (archive_ != nullptr) {
      delete archive_;
      archive_ = nullptr;
    }
  }

  std::ostringstream os_;
  cereal::JSONOutputArchive* archive_;
};

inline net::web_server::string_res_t json_response(
    net::web_server::http_req_t const& req, std::string_view const content) {
  namespace http = boost::beast::http;

  net::web_server::string_res_t res{http::status::ok, req.version()};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, "application/json");
  res.keep_alive(req.keep_alive());
  res.body() = content;
  res.prepare_payload();
  return res;
}

inline net::web_server::string_res_t json_response(
    net::web_server::http_req_t const& req, json_archive& archive) {
  return json_response(req, archive.json());
}

}  // namespace soro::server