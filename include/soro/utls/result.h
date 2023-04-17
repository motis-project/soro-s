#pragma once

#include <expected>
#include <system_error>

#include "fmt/format.h"
#include "utl/logging.h"

#include "soro/utls/sassert.h"

namespace soro::utls {

template <typename T>
using result = std::expected<T, std::error_code>;

template <typename T>
[[nodiscard]] constexpr std::unexpected<typename result<T>::error_type>
propagate(result<T> const& r) {
  utls::expect(!r, "error propagating a valid result");
  utls::expect(r.error() != std::error_code{},
               "unexpected error with no error code");

  return std::unexpected(r.error());
}

template <typename... Args>
[[nodiscard]] inline std::unexpected<std::error_code> unexpected(
    std::error_code const& e, std::string_view const message, Args&&... args) {
  utls::expect(e != std::error_code{}, "unexpected error with no error code");

  uLOG(utl::err) << fmt::format(message, args...);

  return std::unexpected(e);
}

[[nodiscard]] inline std::unexpected<std::error_code> unexpected(
    std::error_code const& e) {
  utls::expect(e != std::error_code{}, "unexpected error with no error code");

  uLOG(utl::err) << fmt::format("error: {}", e.message());

  return std::unexpected(e);
}

}  // namespace soro::utls